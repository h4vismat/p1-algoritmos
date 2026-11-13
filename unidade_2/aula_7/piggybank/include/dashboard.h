#ifndef PIGGYBANK_DASHBOARD_H
#define PIGGYBANK_DASHBOARD_H

#include "journal.h"

typedef enum {
    DASHBOARD_OK = 0,
    DASHBOARD_INPUT_ERROR,
    DASHBOARD_JOURNAL_ERROR,
    DASHBOARD_INVALID_ARGUMENT,
    DASHBOARD_UI_ERROR
} DashboardResult;

DashboardResult dashboard_run(Journal *journal);

#endif
