#include "dashboard.h"
#include "journal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "Uso: %s [arquivo-journal]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *path = argc == 2 ? argv[1] : "piggybank.journal";

    Journal *journal = NULL;
    JournalReport report = {0};

    JournalResult result = journal_open(path,
        JOURNAL_RECOVERY_STRICT,
        &journal,
        &report
    );

    if (result != JOURNAL_OK) {
        fprintf(stderr, "%s", journal_result_string(result));

        if (report.system_errno != 0)
            fprintf(stderr, ": %s", strerror(report.system_errno));

        fputc('\n', stderr);
        return EXIT_FAILURE;
    }

    DashboardResult dashboard_result = dashboard_run(journal);

    int exit_status = dashboard_result == DASHBOARD_OK ? EXIT_SUCCESS : EXIT_FAILURE;
    result = journal_close(journal);

    if (result != JOURNAL_OK) {
        fprintf(stderr, "Fechando journal: %s\n", journal_result_string(result));
        exit_status = EXIT_FAILURE;
    }

    return exit_status;
}
