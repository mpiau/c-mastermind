SRC := src/main.c
SRC += src/mastermind.c
SRC += src/keyboard_inputs.c
SRC += src/console.c
SRC += src/fps_counter.c
SRC += src/widgets/widget_timer.c
SRC += src/widgets/widget_framerate.c
SRC += src/widgets/widget_board.c
SRC += src/widgets/widget_board_summary.c
SRC += src/widgets/widget.c
SRC += src/widgets/widget_utils.c
SRC += src/widgets/widget_border.c
SRC += src/widgets/widget_countdown.c
SRC += src/widgets/widget_board_buttons.c
SRC += src/widgets/widget_screen_size.c
SRC += src/console_screen.c
SRC += src/mouse.c
SRC += src/time_units.c
SRC += src/rect.c
SRC += src/settings.c


CFLAGS += -Iinclude

CC := gcc

# all
.PHONY: all
all: test

.PHONY: clean
clean: clean-test

test: $(SRC)
	$(CC) $(CFLAGS) -g -o $@ $(filter %.c,$^)
	./$@

.PHONY: clean-test
clean-test:
	rm -f test
