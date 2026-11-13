#ifndef TRACKER_H
#define TRACKER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define STEP_GOAL UINT64_C(10000)

typedef enum {
    UNDO_ADDITION,
    UNDO_CORRECTION
} UndoKind;

typedef struct {
    UndoKind kind;
    uint32_t index;
    uint64_t previous_steps;
} UndoEntry;

typedef struct {
    uint64_t *history;
    size_t capacity;

    uint64_t total;
    uint32_t hours;

    uint32_t best_hour;
    uint64_t best_steps;

    double mean;
    double m2;

    UndoEntry *undo;
    size_t undo_count;
    size_t undo_capacity;
} StepTracker;

void tracker_init(StepTracker *tracker);
void tracker_destroy(StepTracker *tracker);

bool tracker_add(StepTracker *tracker, uint64_t steps);
/* User-facing hour: 1 means history[0]. */
bool tracker_correct(StepTracker *tracker, uint32_t hour, uint64_t steps);
bool tracker_undo(StepTracker *tracker);
bool tracker_finished(const StepTracker *tracker);
double tracker_std(const StepTracker *tracker);
uint32_t tracker_estimated_hours(const StepTracker *tracker);

#endif
