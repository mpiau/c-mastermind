SRC := src/main.c
SRC += src/mastermind.c
SRC += src/keyboard_inputs.c
SRC += src/fps_counter.c
SRC += src/random.c
SRC += src/ui.c
SRC += src/mouse.c
SRC += src/time_units.c
SRC += src/rect.c
SRC += src/settings.c
SRC += src/keybindings.c
SRC += src/game/piece.c
SRC += src/terminal/terminal_character.c
SRC += src/terminal/terminal_screen.c
SRC += src/terminal/internal/terminal_sequence.c
SRC += src/terminal/terminal_attributes.c
SRC += src/terminal/terminal_cursor.c
SRC += src/terminal/terminal_colors.c
SRC += src/terminal/terminal_style.c
SRC += src/terminal/terminal.c

SRC += src/events.c
SRC += src/ui/ui.c
SRC += src/requests.c
SRC += src/ui/widgets/widget_bottom_nav.c
SRC += src/ui/widgets/widget_buildversion.c
SRC += src/ui/widgets/widget_framerate.c
SRC += src/ui/widgets/widget_game_board.c
SRC += src/ui/widgets/widget_game_summary.c
SRC += src/ui/widgets/widget_mousepos.c
SRC += src/ui/widgets/widget_peg_selector.c
SRC += src/ui/widgets/widget_peg_tracking.c
SRC += src/ui/widgets/widget_screensize.c
SRC += src/ui/widgets/widget_timer.c

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
