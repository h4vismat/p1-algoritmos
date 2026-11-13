#include "tracker.h"
#include "terminal.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    StepTracker tracker;
    tracker_init(&tracker);

    int result = terminal_run(&tracker);

    if (result == 0) {
        printf(
            "%s depois de %" PRIu32 "hora(s), com %" PRIu64 " passos.\n",
            tracker_finished(&tracker) ? "Objetivo alcançado" : "Sessão finalizada",
            tracker.hours,
            tracker.total
        );
    } else {
        fputs("Falha ao inicializar interface.\n", stderr);
    }

    tracker_destroy(&tracker);

    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
