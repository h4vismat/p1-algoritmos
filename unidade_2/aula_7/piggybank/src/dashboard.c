#include "dashboard.h"
#include "bank.h"
#include "journal.h"

#include <inttypes.h>
#include <locale.h>
#include <notcurses/nckeys.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <notcurses/notcurses.h>

#define MIN_ROWS 18
#define MIN_COLS 72

static int draw_line(
    struct ncplane *plane,
    int row,
    int column,
    const char *text
) {
    unsigned rows;
    unsigned columns;

    ncplane_dim_yx(plane, &rows, &columns);

    if (row < 0 || column < 0 || (unsigned) row >= rows || (unsigned) column > columns)
        return 0;

    size_t available = (size_t)columns - (size_t)column;

    if (available <= 1)
        return 0;

    --available;

    return ncplane_putnstr_yx(plane, row, column, available, text);
}

static bool screen_is_large_enough(struct ncplane *plane) {
    unsigned rows;
    unsigned columns;

    ncplane_dim_yx(plane, &rows, &columns);

    return rows >= MIN_ROWS && columns >= MIN_COLS;
}

static int dashboard_draw(struct ncplane *plane, const BankImage *image, const char *message) {
    ncplane_erase(plane);
    ncplane_set_styles(plane, NCSTYLE_NONE);
    ncplane_set_fg_default(plane);
    ncplane_set_bg_default(plane);

    if (!screen_is_large_enough(plane)) {
        return draw_line(plane, 0, 0, "Ajuste sua tela para 72x18 ou mais. Pressone Q para sair.");
    }

    char line[160];

    if (ncplane_set_fg_rgb(plane, 0x59c2ff) < 0)
        return -1;

    ncplane_set_styles(plane, NCSTYLE_BOLD);

    if (draw_line(plane, 1, 2, "PIGGYBANK") < 0)
        return -1;

    ncplane_set_styles(plane, NCSTYLE_NONE);
    ncplane_set_fg_default(plane);

    uint64_t balance;
    uint64_t coin_count;

    if (!bank_image_balance(image, &balance) || !bank_image_coint_count(image, &coin_count))
        return -1;

    snprintf(line, sizeof line, "Saldo: R$%" PRIu64 ".%02" PRIu64, balance / 100, balance % 100);

    if (draw_line(plane, 3, 2, line) < 0)
        return -1;

    snprintf(line, sizeof line, "Qtd. de moedas: %" PRIu64, coin_count);

    if (draw_line(plane, 4, 2, line) < 0)
        return -1;

    snprintf(line, sizeof line, "%-19s%-24s%s",
             "MOEDA", "QUANTIDADE", "SUBTOTAL");

    if (draw_line(plane, 6, 2, line) < 0)
        return -1;

    static const uint64_t values[3] = {50, 100, 200};
    static const char *labels[3] = {"R$0.50", "R$1.00", "R$2.00"};

    for (size_t i = 0; i < 3; ++i) {
        uint64_t subtotal = image->counts[i] * values[i];

        snprintf(
            line,
            sizeof line,
            "%-19s%-24" PRIu64 "R$%" PRIu64 ".%02" PRIu64,
            labels[i],
            image->counts[i],
            subtotal / 100,
            subtotal % 100
        );

        if (draw_line(plane, 7 + (int)i, 2, line) < 0)
            return -1;
    }

    if (draw_line(plane, 12, 2, message) < 0)
        return -1;

    if (draw_line(
        plane,
        14,
        2,
        "[1] +R$0.50    [2] +R$1.00     [3] +R$2.00"
    ) < 0) return -1;

    return draw_line(plane, 16, 2, "[Q] Sair.");
}

static bool deposit_from_key(
    uint32_t key,
    uint64_t counts[3]
) {
    counts[0] = 0;
    counts[1] = 0;
    counts[2] = 0;

    switch (key) {
        case '1':
            counts[0] = 1;
            return true;

        case '2':
            counts[1] = 1;
            return true;

        case '3':
            counts[2] = 1;
            return true;

        default:
            return false;
    }
}

static const char *deposit_rejection(BankResult result) {
    switch (result) {
        case BANK_OVERFLOW:
            return "Depósito rejeitado: saldo limite atingido.";

        case BANK_EMPTY_TRANSACTION:
            return "Depósito rejeitado: nenhuma moeda selecionada.";

        default:
            return "O banco rejeitou esse depósito";
    }
}

DashboardResult dashboard_run(Journal *journal) {
    if (!journal)
        return DASHBOARD_INVALID_ARGUMENT;

    if (!setlocale(LC_ALL, "")) {
        fprintf(stderr, "Falha ao inicializar o locale.\n");
        return DASHBOARD_UI_ERROR;
    }

    notcurses_options options = {
        .flags = NCOPTION_SUPPRESS_BANNERS
    };

    struct notcurses *nc = notcurses_core_init(&options, stdout);

    if (!nc) {
        fprintf(stderr, "Falha ao inicializar Notcurses.\n");
        return DASHBOARD_UI_ERROR;
    }

    struct ncplane *plane = notcurses_stdplane(nc);

    DashboardResult outcome = DASHBOARD_OK;
    JournalResult journal_error = JOURNAL_OK;
    JournalReport report = {0};
    BankImage image = {0};

    const char *message = "Escolha uma moeda para depositar.";
    const char *failure_message = NULL;

    for (;;) {
        journal_error = journal_snapshot(journal, &image);

        if (journal_error != JOURNAL_OK) {
            outcome = DASHBOARD_JOURNAL_ERROR;
            failure_message = "Falha ao acessar dados bancários.";
            break;
        }

        if (dashboard_draw(plane, &image, message) < 0 || notcurses_render(nc) < 0) {
            outcome = DASHBOARD_UI_ERROR;
            failure_message = "Falha ao exibir dashboard.";
            break;
        }

        ncinput input = {0};
        uint32_t key = notcurses_get_blocking(nc, &input);

        if (key == UINT32_MAX) {
            outcome = DASHBOARD_INPUT_ERROR;
            failure_message = "Falha na entrada do terminal.";
            break;
        }

        if (key == 0)
            continue;

        if (key == NCKEY_EOF)
            break;

        if (key == NCKEY_RESIZE) {
            if (notcurses_refresh(nc, NULL, NULL) < 0) {
                outcome = DASHBOARD_UI_ERROR;
                failure_message = "Falha ao reajustar dashboard.";
                break;
            }

            continue;
        }

        /*
         * NCTYPE_RELEASE and NCTYPE_REPEAT can cause accidental key presses.
         */
        if (input.evtype == NCTYPE_RELEASE || input.evtype == NCTYPE_REPEAT)
            continue;

        if (key == 'q' || key == 'Q')
            break;

        if (!screen_is_large_enough(plane))
            continue;

        uint64_t counts[3];

        if (!deposit_from_key(key, counts)) {
            message = "Use 1, 2 ou 3 para depositar, ou Q para sair.";
            continue;
        }

        report = (JournalReport){0};

        journal_error = journal_commit(journal, TX_DEPOSIT, counts, &report);

        if (journal_error == JOURNAL_OK) {
            message = "Depósito salvo";
        } else if (journal_error == JOURNAL_BANK_REJECTED) {
            message = deposit_rejection(report.bank_result);
        } else {
            outcome = DASHBOARD_JOURNAL_ERROR;
            failure_message = "Estado do depósito incerto.";
            break;
        }
    }

    if (notcurses_stop(nc) < 0) {
        fprintf(stderr, "Falha ao restaurar o terminal.\n");

        if (outcome == DASHBOARD_OK)
            outcome = DASHBOARD_UI_ERROR;
    }

    if (outcome != DASHBOARD_OK) {
        if (failure_message)
            fprintf(stderr, "%s\n", failure_message);

        if (outcome == DASHBOARD_JOURNAL_ERROR) {
            fprintf(stderr, "Journal: %s\n", journal_result_string(journal_error));

            if (report.system_errno != 0) {
                fprintf(stderr, "Sistema: %s\n", strerror(report.system_errno));
            }
        }
    }

    return outcome;
}
