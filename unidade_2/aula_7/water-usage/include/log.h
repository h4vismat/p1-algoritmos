#ifndef LOG_H
#define LOG_H

#include "sensor.h"

#include <stdio.h>

typedef struct Log Log;

typedef enum {
    LOG_OK,
    LOG_NO_MEMORY,
    LOG_IO_ERROR,
    LOG_INVALID_FORMAT,
    LOG_FAILED
} LogResult;

typedef enum {
    LOG_READ_OK,
    LOG_READ_END,
    LOG_READ_IO_ERROR,
    LOG_READ_INVALID_FORMAT,
    LOG_READ_INCOMPLETE
} LogReadResult;

/*
 * Opens an existing log or creates a new one.
 * On failure, sets *out to NULL.
 */
LogResult log_open(const char *path, Log **out);

/*
 * Append a serialized reading to the stream.
 */
LogResult log_append(Log *log, const MeterReading *reading);

/* Flushes the stream buffer to the OS. */
LogResult log_flush(Log *log);

/*
 * Closes the stream and frees the log, even if closing fails.
 * The pointer becomes invalid regardless of the result.
 */
LogResult log_close(Log *log);

/*
 * Borrows input. Writes *out only on success.
 */
LogReadResult log_read_next(FILE *input, MeterReading *out);

#endif
