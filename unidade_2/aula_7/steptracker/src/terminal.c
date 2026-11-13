#include "terminal.h"
#include "tracker.h"

#include <notcurses/nckeys.h>
#include <notcurses/notcurses.h>

#include <inttypes.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BAR_WIDTH 30

typedef enum {
    UI_BROWSING,
    UI_ADDING,
    UI_EDITING
} UiMode;

typedef struct {
    UiMode mode;

    size_t selected;

    // Length of UINT64_MAX is 20 digits.
    char input[21];
    size_t input_length;

    char status[160];
    bool quit;
} UiState;

static bool parse_steps(const char *text, uint64_t *out) {
    if (text[0] < '0' || text[0] > '9')
        return false;

    errno = 0;
    char *end;

    uintmax_t value = strtoumax(text, &end, 10);

    if (errno == ERANGE || *end != '\0' || value > UINT64_MAX)
        return false;

    *out = (uint64_t)value;
    return true;
}

static void set_status(UiState *ui, const char *message) {
    snprintf(ui->status, sizeof(ui->status), "%s", message);
}

static void clamp_selection(UiState *ui, const StepTracker *tracker) {
    if (tracker->hours == 0)
        ui->selected = 0;
    else if (ui->selected >= tracker->hours)
        ui->selected = tracker->hours - 1;
}

static size_t page_size(unsigned rows) {
    return rows > 9 ? (size_t)rows - 9 : 1;
}

static void draw_line(struct ncplane *plane, unsigned row, const char *format, ...) {
    unsigned rows, columns;
    ncplane_dim_yx(plane, &rows, &columns);

    if (row >= rows || columns < 2)
        return;

    char text[256];

    va_list args;
    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);

    size_t length = strlen(text);
    size_t available = (size_t)columns - 1;

    if (length > available)
        length = available;

    // avoid wrapping
    if (length != 0)
        (void)ncplane_putnstr_yx(plane, (int)row, 0, length, text);
}

static void draw(struct ncplane *plane, const StepTracker *tracker, UiState *ui) {
    unsigned rows, columns;
    ncplane_dim_yx(plane, &rows, &columns);

    ncplane_erase(plane);
    ncplane_set_styles(plane, NCSTYLE_NONE);

    clamp_selection(ui, tracker);

    if (rows < 12 || columns < 60) {
        draw_line(plane, 0, "Aumente o terminal para no mínimo 60 x 12.");
        draw_line(plane, 1, "q: sair; Esc: cancelar.");
        return;
    }

    bool finished = tracker_finished(tracker);

    draw_line(
        plane, 0,
        "StepTracker | %s",
        finished ? "OBJETIVO ATINGIDO" : "coletando.."
    );
    draw_line(
        plane, 1,
        "Passos: %" PRIu64 " / %" PRIu64 " | Horas: %" PRIu32,
        tracker->total, STEP_GOAL, tracker->hours
    );

    if (tracker->hours == 0 || tracker->mean <= 0.0) {
        draw_line(
            plane, 2,
            "Média: %.1f | Desvio padrão: %.1f | ETA: indisponível",
            tracker->mean, tracker_std(tracker)
        );
    } else {
        draw_line(
            plane, 2,
            "Média: %.1f | Desvio padrão: %.1f | ETA: Mais %" PRIu32 " horas",
            tracker->mean, tracker_std(tracker), tracker_estimated_hours(tracker)
        );
    }

    size_t per_page = page_size(rows);
    size_t count = tracker->hours;
    size_t start = (ui->selected / per_page) * per_page;

    size_t visible = count > start ? count - start : 0;
    if (visible > per_page)
        visible = per_page;

    size_t pages = count ? (count - 1) / per_page + 1 : 1;

    draw_line(
        plane, 3,
        "Página %zu/%zu",
        start / per_page + 1, pages
    );

    draw_line(plane, 4, "   Horas       Passos      Progresso");

    if (count == 0)
        draw_line(plane, 5, "   Nenhuma hora gravada.");

    for (size_t offset = 0; offset < visible; ++offset) {
        size_t index = start + offset;
        uint64_t steps = tracker->history[index];

        unsigned filled = steps >= STEP_GOAL ? 24 : (unsigned)(((long double)steps / STEP_GOAL) * 24);

        char bar[25];
        memset(bar, '#', filled);
        memset(bar + filled, '-', 24 - filled);
        bar[24] = '\0';

        bool selected = index == ui->selected;

        ncplane_set_styles(plane, selected ? NCSTYLE_BOLD : NCSTYLE_NONE);

        draw_line(
            plane, 5 + (unsigned)offset,
            "%c %5zu  %10" PRIu64 "     [%s]",
            selected ? '>' : ' ',
            index + 1,
            steps,
            bar
        );
    }

    ncplane_set_styles(plane, NCSTYLE_NONE);

    draw_line(
        plane, rows - 4,
        finished
            ? "Up/Down: selecionar | PgUp/PgDown: page | q: sair"
            : "Up/Down: selecionar | PgUp/PgDown: page | a/e/u/q"
    );

    if (ui->mode == UI_ADDING) {
        draw_line(
            plane, rows - 3,
            "Adicionar passos: %s_ [Enter: salvar, Esc: cancelar]",
            ui->input
        );
    } else if (ui->mode == UI_EDITING) {
        draw_line(
            plane, rows - 3,
            "Editar hora %zu: %s_ [Enter: salvar, Esc: cancelar]",
            ui->selected + 1, ui->input
        );
    } else {
        draw_line(
            plane, rows - 3,
            "a: adicionar | e: editar | u: desfazer | q: sair"
        );
    }

    draw_line(plane, rows - 2, "%s", ui->status);
}

static void begin_input(UiState *ui, const StepTracker *tracker, UiMode mode) {
    ui->mode = mode;
    ui->input_length = 0;
    ui->input[0] = '\0';

    if (mode == UI_EDITING) {
        snprintf(ui->input, sizeof(ui->input), "%" PRIu64, tracker->history[ui->selected]);
        ui->input_length = strlen(ui->input);
    }

    set_status(ui, "Esc para cancelar.");
}

static void confirm_input(UiState *ui, StepTracker *tracker) {
    uint64_t steps;

    if(!parse_steps(ui->input, &steps)) {
        set_status(ui, "Digite um número válido.");
        return;
    }

    bool success;

    if (ui->mode == UI_ADDING) {
        success = tracker_add(tracker, steps);
    } else {
        success = tracker_correct(tracker, (uint32_t)(ui->selected + 1), steps);
    }

    if (!success) {
        set_status(ui, "Operação falhou.");
        return;
    }

    if (ui->mode == UI_ADDING)
        ui->selected = tracker->hours - 1;

    ui->mode = UI_BROWSING;
    ui->input_length = 0;
    ui->input[0] = '\0';

    set_status(
        ui,
        tracker_finished(tracker) ? "Objetivo alcançado." : "Salvo."
    );
}

static void handle_input_key(UiState *ui, StepTracker *tracker, uint32_t key) {
    if (key == NCKEY_ESC) {
        ui->mode = UI_BROWSING;
        ui->input_length = 0;
        ui->input[0] = '\0';

        set_status(ui, "Cancelado. Nenhuma mudança aplicada.");
    } else if (key == NCKEY_ENTER || key == '\n' || key == '\r') {
        confirm_input(ui, tracker);
    } else if (key == NCKEY_BACKSPACE) {
        if (ui->input_length != 0)
            ui->input[--ui->input_length] = '\0';
    } else if (key >= '0' && key <= '9') {
        ui->input[ui->input_length++] = (char)key;
        ui->input[ui->input_length] = '\0';
    }
}

static void handle_browsing_key(
    UiState *ui,
    StepTracker *tracker,
    uint32_t key,
    size_t per_page
) {
    clamp_selection(ui, tracker);

    if (key == 'q') {
        ui->quit = true;
        return;
    }

    if (key == NCKEY_UP) {
        if (ui->selected != 0)
            ui->selected--;

        return;
    }

    if (key == NCKEY_DOWN) {
        if (tracker->hours != 0 && ui->selected < (size_t)tracker->hours - 1)
            ui->selected++;

        return;
    }

    if (key == NCKEY_PGUP) {
        ui->selected = ui->selected >= per_page
            ? ui->selected - per_page
            : 0;

        return;
    }

    if (key == NCKEY_PGDOWN) {
        if (tracker->hours != 0) {
            size_t remaining = (size_t)tracker->hours - 1 - ui->selected;
            ui->selected += remaining < per_page ? remaining : per_page;
        }

        return;
    }

    if (key != 'a' && key != 'e' && key != 'u')
        return;

    if (tracker_finished(tracker)) {
        set_status(ui, "Objetivo alcançado, modo de apenas leitura.");
        return;
    }

    switch (key) {
        case 'a':
            begin_input(ui, tracker, UI_ADDING);
            break;

        case 'e':
            if (tracker->hours == 0)
                set_status(ui, "Sem histórico para editar.");
            else
                begin_input(ui, tracker, UI_EDITING);
            break;

        case 'u':
            if (tracker_undo(tracker)) {
                clamp_selection(ui, tracker);
                set_status(ui, "Ultima mudança desfeita.");
            } else {
                set_status(ui, "Nada a desfazer.");
            }
            break;
    }
}

int terminal_run(StepTracker *tracker) {
    UiState ui = {0};
    set_status(&ui, "Aperte 'a' para inserir o primeiro registro.");

    notcurses_options options = {
        .flags = NCOPTION_SUPPRESS_BANNERS
    };

    struct notcurses *nc = notcurses_core_init(&options, stdout);

    if (nc == NULL) {
        fputs("Falha ao inicializar Notcurses.\n", stderr);
        return 1;
    }

    struct ncplane *plane = notcurses_stdplane(nc);

    while (!ui.quit) {
        draw(plane, tracker, &ui);

        if (notcurses_render(nc) != 0) {
            return 1;
        }

        ncinput input = {0};
        uint32_t key = notcurses_get_blocking(nc, &input);

        if (key == UINT32_MAX) {
            if (errno == EINTR)
                continue;

            return 1;
        }

        if (key == NCKEY_EOF)
            return 0;

        if (input.evtype == NCTYPE_RELEASE || key == 0)
            continue;

        if (key == NCKEY_RESIZE) {
            if (notcurses_refresh(nc, NULL, NULL) < 0) {
                return 1;
            }

            continue;
        }

        unsigned rows, columns;
        ncplane_dim_yx(plane, &rows, &columns);

        if (rows < 12 || columns < 60) {
            if (key == NCKEY_ESC)
                ui.mode = UI_BROWSING;
            else if (key == 'q' && ui.mode == UI_BROWSING)
                ui.quit = true;
            continue;
        }

        if (ui.mode == UI_BROWSING)
            handle_browsing_key(&ui, tracker, key, page_size(rows));
        else
            handle_input_key(&ui, tracker, key);
    }

    if (notcurses_stop(nc) != 0)
        return 1;

    return 0;
}
