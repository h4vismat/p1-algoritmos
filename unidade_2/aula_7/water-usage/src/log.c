#include "log.h"
#include "sensor.h"

#include <errno.h>
#include <inttypes.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct Log {
    FILE *file;
    bool failed;
};

static bool parse_unsigned(
    const char **cursor,
    uintmax_t maximum,
    uint64_t *out
) {
    const char *start = *cursor;

    if (*start < '0' || *start > '9')
        return false;

    errno = 0;
    char *end;
    uintmax_t value = strtoumax(start, &end, 10);

    if (errno == ERANGE || value > maximum || *end != ',')
        return false;

    *out = (uint64_t)value;
    *cursor = end + 1;
    return true;
}

static bool parse_reading(const char *line, MeterReading *out) {
    MeterReading reading;
    uint64_t sensor_id;
    const char *cursor = line;

    if (!parse_unsigned(&cursor, UINT32_MAX, &sensor_id) ||
        !parse_unsigned(&cursor, UINT64_MAX, &reading.generation) ||
        !parse_unsigned(&cursor, UINT64_MAX, &reading.sequence) ||
        !parse_unsigned(&cursor, UINT64_MAX, &reading.occurred_at_ms)) {
        return false;
    }

    // volume must begin with digit or minus sign
    if ((*cursor < '0' || *cursor > '9') && *cursor != '-')
        return false;

    errno = 0;
    char *end;
    double volume = strtod(cursor, &end);

    if (end == cursor || *end != '\0' || !isfinite(volume) || volume < 0.0 || (errno == ERANGE && volume == 0.0))
        return false;

    reading.sensor_id = (uint32_t)sensor_id;
    reading.total_liters = volume;

    *out = reading;
    return true;
}

LogReadResult log_read_next(FILE *input, MeterReading *out) {
    char line[256];
    size_t length = 0;

    for (;;) {
        int ch = fgetc(input);

        if (ch == EOF) {
            if (ferror(input))
                return LOG_READ_IO_ERROR;

            return length == 0 ? LOG_READ_END : LOG_READ_INCOMPLETE;
        }

        if (ch == '\n')
            break;

        if (ch == '\0' || length == sizeof line - 1)
            return LOG_READ_INVALID_FORMAT;
    }

    if (length > 0 && line[length - 1] == '\r')
        length--;

    line[length] = '\0';

    MeterReading reading;
    if (!parse_reading(line, &reading))
        return LOG_READ_INVALID_FORMAT;

    *out = reading;
    return LOG_READ_OK;
}

LogResult log_open(const char *path, Log **out) {
    *out = NULL;

    Log *log = malloc(sizeof *log);
    if (log == NULL)
        return LOG_NO_MEMORY;

    FILE *file = fopen(path, "a");

    if (file == NULL) {
        free(log);
        return LOG_IO_ERROR;
    }

    *log = (Log){
        .file = file,
        .failed = false
    };

    *out = log;
    return LOG_OK;
}

LogResult log_append(Log *log, const MeterReading *reading) {
    if (log->failed)
        return LOG_FAILED;

    int result = fprintf(
        log->file,
        "%" PRIu32 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%.*g\n",
        reading->sensor_id,
        reading->generation,
        reading->sequence,
        reading->occurred_at_ms,
        DBL_DECIMAL_DIG,
        reading->total_liters
    );

    if (result < 0) {
        log->failed = true;
        return LOG_IO_ERROR;
    }

    return LOG_OK;
}

LogResult log_flush(Log *log) {
    if (log->failed)
        return LOG_FAILED;

    if (fflush(log->file) == EOF) {
        log->failed = true;
        return LOG_IO_ERROR;
    }

    return LOG_OK;
}

LogResult log_close(Log *log) {
    if (log == NULL)
        return LOG_OK;

    LogResult result = log->failed ? LOG_FAILED : LOG_OK;

    if (fclose(log->file) == EOF)
        result = LOG_IO_ERROR;

    free(log);
    return result;
}
