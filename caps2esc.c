#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <linux/input.h>

// clang-format off
const struct input_event
esc_up          = {.type = EV_KEY, .code = KEY_ESC,      .value = 0},
ctrl_up         = {.type = EV_KEY, .code = KEY_LEFTCTRL, .value = 0},
capslock_up     = {.type = EV_KEY, .code = KEY_CAPSLOCK, .value = 0},
esc_down        = {.type = EV_KEY, .code = KEY_ESC,      .value = 1},
ctrl_down       = {.type = EV_KEY, .code = KEY_LEFTCTRL, .value = 1},
capslock_down   = {.type = EV_KEY, .code = KEY_CAPSLOCK, .value = 1},
esc_repeat      = {.type = EV_KEY, .code = KEY_ESC,      .value = 2},
ctrl_repeat     = {.type = EV_KEY, .code = KEY_LEFTCTRL, .value = 2},
capslock_repeat = {.type = EV_KEY, .code = KEY_CAPSLOCK, .value = 2},
syn             = {.type = EV_SYN, .code = SYN_REPORT,   .value = 0};
// clang-format on

int equal(const struct input_event *first, const struct input_event *second) {
    return first->type == second->type && first->code == second->code &&
           first->value == second->value;
}

int read_event(struct input_event *event) {
    return fread(event, sizeof(struct input_event), 1, stdin) == 1;
}

void write_event(const struct input_event *event) {
    if (fwrite(event, sizeof(struct input_event), 1, stdout) != 1)
        exit(EXIT_FAILURE);
}

int main(void) {
    int capslock_is_down = 0, esc_give_up = 0, control_is_down = 0;
    struct input_event input;

    setbuf(stdin, NULL), setbuf(stdout, NULL);

    while (read_event(&input)) {
        if (input.type == EV_MSC && input.code == MSC_SCAN)
            continue;

        if (input.type != EV_KEY) {
            write_event(&input);
            continue;
        }

        if (capslock_is_down || control_is_down) {
            if (equal(&input, &capslock_down) ||
                equal(&input, &capslock_repeat) ||
                equal(&input, &ctrl_down) ||
                equal(&input, &ctrl_repeat))
                continue;

            if (equal(&input, &capslock_up) ||
                equal(&input, &ctrl_up)) {
                capslock_is_down = 0;
                control_is_down = 0;
                if (esc_give_up) {
                    esc_give_up = 0;
                    write_event(&ctrl_up);
                    continue;
                }
                write_event(&esc_down);
                write_event(&syn);
                usleep(20000);
                write_event(&esc_up);
                continue;
            }

            if (!esc_give_up && input.value) {
                esc_give_up = 1;
                write_event(&ctrl_down);
                write_event(&syn);
                usleep(20000);
            }
        } else if (equal(&input, &capslock_down)) {
            capslock_is_down = 1;
            continue;
        } else if (equal(&input, &ctrl_down)) {
            control_is_down = 1;
            continue;
        }

        write_event(&input);
    }
}
