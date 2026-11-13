#ifndef PIGGYBANK_JOURNAL_H
#define PIGGYBANK_JOURNAL_H

#include "bank.h"

#include <stdint.h>
#include <sys/types.h>

typedef struct Journal Journal;

typedef enum {
    JOURNAL_OK = 0,
    JOURNAL_INVALID_ARGUMENT,
    JOURNAL_NO_MEMORY,
    JOURNAL_IO_ERROR,
    JOURNAL_LOCKED,
    JOURNAL_CORRUPT,
    JOURNAL_UNSUPPORTED_VERSION,
    JOURNAL_INCOMPLETE_TAIL,
    JOURNAL_BANK_REJECTED,
    JOURNAL_NEEDS_RECOVERY,
    JOURNAL_BAD_SEQUENCE,
    JOURNAL_THREAD_ERROR,
    JOURNAL_INVARIANT_ERROR
} JournalResult;

typedef enum {
    JOURNAL_RECOVERY_STRICT = 0,

    /* Explicitly permit discarding an incomplete final record. */
    JOURNAL_RECOVER_TRUNCATE_TAIL
} JournalRecovery;

typedef struct {
    // OS related failures.
    int system_errno;

    // Domain rejection.
    BankResult bank_result;

    // End of the last complete and accepted record during recovery.
    off_t valid_bytes;

    // Bytes removed by explicitly requested tail repair.
    off_t discarded_bytes;
} JournalReport;

/*
 * Opens or creates the file, acquires a lock and replays it.
 * On success:
 * - *out owns a ready journal.
 * - the journal owns its reconstructed Bank.
 *
 * On failure:
 * - *out is NULL.
 * - resources are released.
 *
 * report may be NULL.
 */
JournalResult journal_open(
    const char *path,
    JournalRecovery recovery,
    Journal **out,
    JournalReport *report
);

/*
 * Assigns the next sequence number, validates, persists and applies
 * one complete transaction.
 */
JournalResult journal_commit(
    Journal *journal,
    TxKind kind,
    const uint64_t counts[3],
    JournalReport *report
);

/*
 * Copies a consistend bank snapshot into out.
 * Leaves out unchanged on failure.
 */
JournalResult journal_snapshot(Journal *journal, BankImage *out);

/*
 * Released the bank, descriptors, and lock.
 *
 * Frees the journal even when close reports an error.
 */
JournalResult journal_close(Journal *journal);

const char *journal_result_string(JournalResult result);

#endif
