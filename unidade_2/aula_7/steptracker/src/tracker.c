#include "tracker.h"

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

void tracker_init(StepTracker *tracker) {
    *tracker = (StepTracker){0};
}

void tracker_destroy(StepTracker *tracker) {
    free(tracker->history);
    free(tracker->undo);

    tracker_init(tracker);
}

/*
 * Ensure space for one more reading.
 *
 * Success may move history, invalidating pointers into the old array.
 */
static bool reserve_next_hour(StepTracker *tracker) {
    if (tracker->hours < tracker->capacity)
        return true;

    const size_t maximum = SIZE_MAX / sizeof(*tracker->history);

    if (tracker->capacity >= maximum)
        return false;

    size_t new_capacity;

    if (tracker->capacity == 0) {
        new_capacity = maximum < 8 ? maximum : 8;
    } else if (tracker->capacity > maximum / 2) {
        new_capacity = maximum;
    } else {
        new_capacity = tracker->capacity * 2;
    }

    uint64_t *grown = realloc(tracker->history, new_capacity * sizeof(*grown));

    if (grown == NULL)
        return false;

    tracker->history = grown;
    tracker->capacity = new_capacity;

    return true;
}

static bool reserve_undo_entry(StepTracker *tracker) {
    if (tracker->undo_count < tracker->undo_capacity)
        return true;

    const size_t maximum = SIZE_MAX / sizeof(*tracker->undo);

    if (tracker->undo_capacity >= maximum)
        return false;

    size_t new_capacity;

    if (tracker->undo_capacity == 0) {
        new_capacity = maximum < 8 ? maximum : 8;
    } else if (tracker->undo_capacity > maximum / 2) {
        new_capacity = maximum;
    } else {
        new_capacity = tracker->undo_capacity * 2;
    }

    UndoEntry *grown = realloc(tracker->undo, new_capacity * sizeof(*grown));

    if (grown == NULL)
        return false;

    tracker->undo = grown;
    tracker->undo_capacity = new_capacity;

    return true;
}

static void rebuild_statistics(StepTracker *tracker) {
    tracker->total = 0;
    tracker->best_hour = 0;
    tracker->best_steps = 0;
    tracker->mean = 0.0;
    tracker->m2 = 0.0;

    for (uint32_t i = 0; i < tracker->hours; ++i) {
        uint64_t steps = tracker->history[i];

        tracker->total += steps;

        if (i == 0 || steps > tracker->best_steps) {
            tracker->best_steps = steps;
            tracker->best_hour = i + 1;
        }

        double count = (double)i + 1.0;
        double delta = (double)steps - tracker->mean;

        tracker->mean += delta / count;

        double delta2 = (double)steps - tracker->mean;
        tracker->m2 += delta * delta2;
    }
}

bool tracker_finished(const StepTracker *tracker) {
    return tracker->total >= STEP_GOAL;
}

bool tracker_add(StepTracker *tracker, uint64_t steps) {
    if (tracker_finished(tracker) || tracker->hours == UINT32_MAX || steps > UINT64_MAX - tracker->total) {
        return false;
    }

    if (!reserve_undo_entry(tracker))
        return false;

    if (!reserve_next_hour(tracker))
        return false;

    tracker->undo[tracker->undo_count++] = (UndoEntry){
        .kind = UNDO_ADDITION,
        .index = tracker->hours,
        .previous_steps = 0
    };

    tracker->history[tracker->hours++] = steps;

    rebuild_statistics(tracker);
    return true;
}

bool tracker_correct(StepTracker *tracker, uint32_t hour, uint64_t steps) {
    if (tracker_finished(tracker))
        return false;

    if (hour == 0 || hour > tracker->hours)
        return false;

    uint32_t index = hour - 1;
    uint64_t previous = tracker->history[index];

    // Successful but no undo entry.
    if (steps == previous)
        return true;

    uint64_t other_steps = tracker->total - previous;

    if (steps > UINT64_MAX - other_steps)
        return false;

    if (!reserve_undo_entry(tracker))
        return false;

    tracker->undo[tracker->undo_count++] = (UndoEntry){
        .kind = UNDO_CORRECTION,
        .index = index,
        .previous_steps = previous
    };

    tracker->history[index] = steps;

    rebuild_statistics(tracker);
    return true;
}

bool tracker_undo(StepTracker *tracker) {
    if (tracker_finished(tracker))
        return false;

    if (tracker->undo_count == 0)
        return false;

    UndoEntry entry = tracker->undo[--tracker->undo_count];

    switch (entry.kind) {
        case UNDO_ADDITION:
            tracker->hours--;
            break;

        case UNDO_CORRECTION:
            tracker->history[entry.index] = entry.previous_steps;
    }

    rebuild_statistics(tracker);
    return true;
}

double tracker_variance(const StepTracker *tracker) {
    return tracker->hours ? tracker->m2 / tracker->hours : 0.0;
}

double tracker_std(const StepTracker *tracker) {
    return sqrt(tracker_variance(tracker));
}

uint32_t tracker_estimated_hours(const StepTracker *tracker) {
    if (tracker_finished(tracker)) return 0;
    if (tracker->hours == 0 || tracker->mean <= 0.0) return 0;

    double needed = ceil((double)(STEP_GOAL - tracker->total) / tracker->mean);
    if (needed > UINT_MAX) return UINT_MAX;
    return (uint32_t)needed;
}
