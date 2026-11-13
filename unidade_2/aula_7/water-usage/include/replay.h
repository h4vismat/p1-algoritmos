#ifndef REPLAY_H
#define REPLAY_H

#include "monitor.h"
#include "residence.h"

#include <stdio.h>

typedef enum {
    REPLAY_OK,
    REPLAY_IO_ERROR,
    REPLAY_INVALID_FORMAT,
    REPLAY_INCOMPLETE_LOG,
    REPLAY_UNKNOWN_SENSOR,
    REPLAY_UNASSIGNED_SENSOR,
    REPLAY_INVALID_READING,
    REPLAY_STORE_ERROR
} ReplayResult;

/*
 * Reads from input's current position until EOF.
 *
 * Preconditions:
 * - monitor and residences contain configuration but no readings/history.
 * - input is positioned at first record.
 * - no concurrent ingestion or store mutation occurs.
 *
 * On failure, state may be partially rebuilt:
 * the caller must discard monitor and resi
 */
ReplayResult application_replay(FILE *input, Monitor *monitor, ResidenceStore *residences);

#endif
