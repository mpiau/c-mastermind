SRC := src/main.c
SRC += src/mastermind.c
SRC += src/keyboard_inputs.c
SRC += src/fps_counter.c
SRC += src/random.c
SRC += src/mouse.c
SRC += src/time_units.c
SRC += src/rect.c
SRC += src/settings.c
SRC += src/components/component_framerate.c
SRC += src/components/component_game_buttons.c
SRC += src/components/component_mouse_position.c
SRC += src/components/component_screen_size.c
SRC += src/components/component_summary.c
SRC += src/components/component_timer.c
SRC += src/components/components.c
SRC += src/widgets/widget_board.c
SRC += src/widgets/widget_countdown.c
SRC += src/widgets/widget_peg_selection.c
SRC += src/game/piece.c
SRC += src/terminal/terminal_character.c
SRC += src/terminal/terminal_screen.c
SRC += src/terminal/internal/terminal_sequence.c
SRC += src/terminal/terminal_attributes.c
SRC += src/terminal/terminal_cursor.c
SRC += src/terminal/terminal_colors.c
SRC += src/terminal/terminal_style.c
SRC += src/terminal/terminal.c


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
