#include "journal.h"
#include "bank.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#define RECORD_SIZE 42
#define CHECKSUM_OFFSET 38
#define FORMAT_VERSION 1

struct Journal {
    int fd;
    int directory_fd;

    Bank *bank;
    bool ready;

    pthread_mutex_t mutex;
    bool mutex_initialized;
};

static void clear_report(JournalReport *report) {
    *report = (JournalReport){
        .bank_result = BANK_OK
    };
}

static void put_le(
    unsigned char *destination,
    uint64_t value,
    size_t bytes
) {
    for (size_t i = 0; i < bytes; ++i) {
        destination[i] = (unsigned char)(value & UINT64_C(0xff));
        value >>= 8;
    }
}

static uint64_t get_le(
    const unsigned char *source,
    size_t bytes
) {
    uint64_t value = 0;

    for (size_t i = 0; i < bytes; ++i)
        value |= (uint64_t)source[i] << (8 * i);

    return value;
}

static uint32_t crc32(
    const unsigned char *data,
    size_t length
) {
    uint32_t crc = UINT32_MAX;

    for (size_t i = 0; i < length; ++i) {
        crc ^= (uint32_t)data[i];

        for (unsigned bit = 0; bit < 8; ++bit) {
            if (crc & UINT32_C(1))
                crc = (crc >> 1) ^ UINT32_C(0xedb88320);
            else
                crc >>= 1;
        }
    }

    return crc ^ UINT32_MAX;
}

static void encode_record(
    const Transaction *tx,
    unsigned char out[RECORD_SIZE]
) {
    memcpy(out, "PBTX", 4);
    out[4] = FORMAT_VERSION;

    put_le(out + 5, tx->sequence, 8);
    out[13] = tx->kind == TX_DEPOSIT ? 1 : 2;

    for (size_t i = 0; i < 3; ++i) {
        put_le(out + 14 + 8 * i, tx->counts[i], 8);
    }

    put_le(out + CHECKSUM_OFFSET, crc32(out, CHECKSUM_OFFSET), 4);
}

static JournalResult decode_record(
    const unsigned char in[RECORD_SIZE],
    Transaction *tx
) {
    if (memcmp(in, "PBTX", 4) != 0)
        return JOURNAL_CORRUPT;

    if (in[4] != FORMAT_VERSION)
        return JOURNAL_UNSUPPORTED_VERSION;

    uint32_t stored_crc = (uint32_t)get_le(in + CHECKSUM_OFFSET, 4);
    if (stored_crc != crc32(in, CHECKSUM_OFFSET))
        return JOURNAL_CORRUPT;

    if (in[13] != 1 && in[13] != 2)
        return JOURNAL_CORRUPT;

    *tx = (Transaction){
        .sequence = get_le(in + 5, 8),
        .kind = in[13] == 1 ? TX_DEPOSIT : TX_WITHDRAW
    };

    for (size_t i = 0; i < 3; ++i) {
        tx->counts[i] = get_le(in + 14 + 8 * i, 8);
    }

    return JOURNAL_OK;
}

static bool sync_fd(int fd) {
    for (;;) {
        if (fsync(fd) == 0) return true;
        if (errno != EINTR) return false;
    }
}

static bool write_all(
    int fd,
    const unsigned char *data,
    size_t length
) {
    size_t written = 0;

    while (written < length) {
        ssize_t n = write(fd, data + written, length - written);

        if (n < 0) {
            if (errno == EINTR) continue;

            return false;
        }

        if (n == 0) {
            errno = EIO;
            return false;
        }

        written += (size_t)n;
    }

    return true;
}

/*
 * Returns:
 *  1: Complete record.
 *  0: EOF, possibly after reading an incomplete record.
 *  -1: I/O error.
 */
static int read_record(
    int fd,
    unsigned char out[RECORD_SIZE],
    size_t *received
) {
    *received = 0;

    while (*received < RECORD_SIZE) {
        ssize_t n = read(fd, out + *received, RECORD_SIZE - *received);

        if (n < 0) {
            if (errno == EINTR) continue;

            return -1;
        }

        if (n == 0) return 0;

        *received += (size_t)n;
    }

    return 1;
}

static JournalResult open_descriptors(
    Journal *journal,
    const char *path,
    JournalReport *report
) {
    char *copy = strdup(path);
    if (!copy) return JOURNAL_NO_MEMORY;

    char *separator = strrchr(copy, '/');

    const char *directory = ".";
    const char *filename = copy;

    if (separator) {
        filename = separator + 1;

        if (separator == copy) {
            directory = "/";
        } else {
            *separator = '\0';
            directory = copy;
        }
    }

    if (filename[0] == '\0' ||
        strcmp(filename, ".") == 0 ||
        strcmp(filename, "..") == 0) {
            free(copy);
            return JOURNAL_INVALID_ARGUMENT;
        }

    journal->directory_fd = open(directory, O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    if (journal->directory_fd < 0) {
        int saved_errno = errno;
        free(copy);
        report->system_errno = saved_errno;
        return JOURNAL_IO_ERROR;
    }

    journal->fd = openat(
        journal->directory_fd,
        filename,
        O_RDWR | O_CREAT | O_APPEND | O_CLOEXEC | O_NOFOLLOW,
        0600
    );

    int saved_errno = errno;
    free(copy);

    if (journal->fd < 0) {
        report->system_errno = saved_errno;
        return JOURNAL_IO_ERROR;
    }

    struct stat info;
    if (fstat(journal->fd, &info) < 0) {
        report->system_errno = errno;
        return JOURNAL_IO_ERROR;
    }

    if (!S_ISREG(info.st_mode)) return JOURNAL_INVALID_ARGUMENT;

    for (;;) {
        if (flock(journal->fd, LOCK_EX | LOCK_NB) == 0)
            break;

        if (errno == EINTR)
            continue;

        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            report->system_errno = errno;
            return JOURNAL_LOCKED;
        }

        report->system_errno = errno;
        return JOURNAL_IO_ERROR;
    }

    return JOURNAL_OK;
}

static JournalResult replay(
    Journal *journal,
    JournalRecovery recovery,
    JournalReport *report
) {
    if (lseek(journal->fd, 0, SEEK_SET) == (off_t)-1) {
        report->system_errno = errno;
        return JOURNAL_IO_ERROR;
    }

    unsigned char record[RECORD_SIZE];

    for (;;) {
        size_t received = 0;
        int status = read_record(journal->fd, record, &received);

        if (status < 0) {
            report->system_errno = errno;
            return JOURNAL_IO_ERROR;
        }

        if (status == 0) {
            if (received == 0) break;

            if (recovery == JOURNAL_RECOVERY_STRICT) return JOURNAL_INCOMPLETE_TAIL;

            /*
             * Only an incomplete final record reaches this branch.
             */
            if (ftruncate(journal->fd, report->valid_bytes) < 0) {
                report->system_errno = errno;
                return JOURNAL_IO_ERROR;
            }

            report->discarded_bytes = (off_t)received;
            break;
        }

        Transaction tx;
        JournalResult result = decode_record(record, &tx);

        if (result != JOURNAL_OK) return result;

        BankResult bank_result = bank_apply(journal->bank, &tx);
        if (bank_result != BANK_OK) {
            report->bank_result = bank_result;
            return JOURNAL_CORRUPT;
        }

        off_t position = lseek(journal->fd, 0, SEEK_CUR);
        if (position == (off_t)-1) {
            report->system_errno = errno;
            return JOURNAL_IO_ERROR;
        }

        report->valid_bytes = position;
    }

    /*
     * Sync both:
     *  a) accepted records or a repaired file length;
     *  b) the directory entry, including a newly created journal.
     */
    if (!sync_fd(journal->fd)) {
        report->system_errno = errno;
        return JOURNAL_IO_ERROR;
    }

    if (!sync_fd(journal->directory_fd)) {
        report->system_errno = errno;
        return JOURNAL_IO_ERROR;
    }

    journal->ready = true;
    return JOURNAL_OK;
}

JournalResult journal_open(
    const char *path,
    JournalRecovery recovery,
    Journal **out,
    JournalReport *report
) {
    JournalReport local_report;
    if (!report)
        report = &local_report;

    clear_report(report);

    if (!out)
        return JOURNAL_INVALID_ARGUMENT;

    *out = NULL;

    if (!path || path[0] == '\0')
        return JOURNAL_INVALID_ARGUMENT;

    if (recovery != JOURNAL_RECOVERY_STRICT && recovery != JOURNAL_RECOVER_TRUNCATE_TAIL)
        return JOURNAL_INVALID_ARGUMENT;

    Journal *journal = calloc(1, sizeof(*journal));
    if (!journal)
        return JOURNAL_NO_MEMORY;

    journal->fd = -1;
    journal->directory_fd = -1;

    int error = pthread_mutex_init(&journal->mutex, NULL);
    if (error != 0) {
        report->system_errno = error;
        free(journal);
        return JOURNAL_THREAD_ERROR;
    }

    journal->mutex_initialized = true;

    JournalResult result = open_descriptors(journal, path, report);

    if (result != JOURNAL_OK) {
        (void)journal_close(journal);
        return result;
    }

    journal->bank = bank_create();
    if (!journal->bank) {
        (void)journal_close(journal);
        return JOURNAL_NO_MEMORY;
    }

    result = replay(journal, recovery, report);
    if (result != JOURNAL_OK) {
        (void)journal_close(journal);
        return result;
    }

    *out = journal;
    return JOURNAL_OK;
}

JournalResult journal_commit_locked(
    Journal *journal,
    TxKind kind,
    const uint64_t counts[3],
    JournalReport *report
) {
    JournalReport local_report;
    if (!report)
        report = &local_report;

    clear_report(report);

    if (!journal || !counts)
        return JOURNAL_INVALID_ARGUMENT;

    if (!journal->ready)
        return JOURNAL_NEEDS_RECOVERY;

    uint64_t previous_seq = bank_sequence(journal->bank);

    Transaction tx = {
        .sequence = previous_seq + 1,
        .kind = kind,
        .counts = { counts[0], counts[1], counts[2] }
    };

    BankResult bank_result = bank_validate(journal->bank, &tx);

    if (bank_result != BANK_OK) {
        report->bank_result = bank_result;
        return JOURNAL_BANK_REJECTED;
    }

    unsigned char record[RECORD_SIZE];
    encode_record(&tx, record);

    /*
     * Disallow other operations after uncertain I/O.
     * Restored only after persistence and application succeed.
     */
    journal->ready = false;
    if (!write_all(journal->fd, record, sizeof(record))) {
        report->system_errno = errno;
        return JOURNAL_IO_ERROR;
    }

    if (!sync_fd(journal->fd)) {
        report->system_errno = errno;
        return JOURNAL_IO_ERROR;
    }

    bank_result = bank_apply(journal->bank, &tx);
    if (bank_result != BANK_OK) {
        report->bank_result = bank_result;
        return JOURNAL_INVARIANT_ERROR;
    }

    journal->ready = true;
    return JOURNAL_OK;
}

JournalResult journal_commit(Journal *journal, TxKind kind, const uint64_t counts[3], JournalReport *report) {
    JournalReport local_report;
    if (!report)
        report = &local_report;

    clear_report(report);

    if (!journal || !counts)
        return JOURNAL_INVALID_ARGUMENT;

    int error = pthread_mutex_lock(&journal->mutex);
    if (error != 0) {
        report->system_errno = error;
        return JOURNAL_THREAD_ERROR;
    }

    JournalResult result = journal_commit_locked(journal, kind, counts, report);

    error = pthread_mutex_unlock(&journal->mutex);

    if (error != 0) abort();

    return result;
}

JournalResult journal_snapshot(Journal *journal, BankImage *out) {
    if (!journal || !out)
        return JOURNAL_INVALID_ARGUMENT;

    int error = pthread_mutex_lock(&journal->mutex);
    if (error != 0)
        return JOURNAL_THREAD_ERROR;

    JournalResult result = JOURNAL_OK;

    if (!journal->ready) {
        result = JOURNAL_NEEDS_RECOVERY;
    } else {
        bank_export(journal->bank, out);
    }

    if (pthread_mutex_unlock(&journal->mutex) != 0)
        abort();

    return result;
}

JournalResult journal_close(Journal *journal) {
    if(!journal) return JOURNAL_OK;

    if (journal->mutex_initialized) {
        int error = pthread_mutex_destroy(&journal->mutex);
        if (error != 0)
            abort();
    }

    int first_error = 0;

    if (journal->fd >= 0) {
        if (close(journal->fd) < 0) first_error = errno;
    }

    if (journal->directory_fd >= 0) {
        if (close(journal->directory_fd) < 0 && first_error == 0) {
            first_error = errno;
        }
    }

    bank_destroy(journal->bank);
    free(journal);

    if (first_error != 0) {
        errno = first_error;
        return JOURNAL_IO_ERROR;
    }

    return JOURNAL_OK;
}

const char *journal_result_string(JournalResult result) {
    switch (result) {
        case JOURNAL_OK:
            return "success";

        case JOURNAL_INVALID_ARGUMENT:
            return "invalid argument or file type";

        case JOURNAL_NO_MEMORY:
            return "out of memory";

        case JOURNAL_IO_ERROR:
            return "storage operation failed.";

        case JOURNAL_LOCKED:
            return "journal is already in use";

        case JOURNAL_CORRUPT:
            return "journal contains an invalid record";

        case JOURNAL_UNSUPPORTED_VERSION:
            return "unsupported journal format";

        case JOURNAL_INCOMPLETE_TAIL:
            return "journal ends with an incomplete record";

        case JOURNAL_BANK_REJECTED:
            return "bank rejected the transaction";

        case JOURNAL_NEEDS_RECOVERY:
            return "journal must be closed and recovered";

        case JOURNAL_INVARIANT_ERROR:
            return "durable transaction could not be applied";

        case JOURNAL_BAD_SEQUENCE:
            return "sequence is invalid";

        case JOURNAL_THREAD_ERROR:
            return "thread failed";
    }

    return "unknown journal error";
}
