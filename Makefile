SRC := src/main.c
SRC += src/mastermind.c
SRC += src/keyboard_inputs.c
SRC += src/console/console.c
SRC += src/fps_counter.c
SRC += src/components/component_framerate.c
SRC += src/components/component_game_buttons.c
SRC += src/components/component_mouse_position.c
SRC += src/components/component_screen_size.c
SRC += src/components/component_summary.c
SRC += src/widgets/widget.c
SRC += src/widgets/widget_board.c
SRC += src/widgets/widget_timer.c
SRC += src/widgets/widget_utils.c
SRC += src/widgets/widget_countdown.c
SRC += src/widgets/widget_peg_selection.c
SRC += src/mouse.c
SRC += src/time_units.c
SRC += src/rect.c
SRC += src/settings.c
SRC += src/game/piece.c
SRC += src/terminal/terminal_character.c
SRC += src/terminal/terminal_screen.c
SRC += src/terminal/terminal_sequence.c
SRC += src/terminal/terminal_style.c


CFLAGS += -Iinclude

CC := gcc

# all
.PHONY: all
all: test

.PHONY: clean
clean: clean-test

test: $(SRC)
	$(CC) $(CFLAGS) -DDEBUG -g -o $@ $(filter %.c,$^)
	./$@

.PHONY: clean-test
clean-test:
	rm -f test
