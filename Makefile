SRC := src/main.c
SRC += src/mastermind.c
SRC += src/game_menus.c
SRC += src/keyboard_inputs.c
SRC += src/menu_stack.c
SRC += src/console.c
SRC += src/fps_counter.c
SRC += src/mastermind_v2.c
SRC += src/widgets/widget_timer.c
SRC += src/widgets/widget_fpsbar.c
SRC += src/widgets/widgets.c


CFLAGS += -Iinclude

CC := gcc

# all
.PHONY: all
all: test

.PHONY: clean
clean: clean-test

test: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^)
	./$@

.PHONY: clean-test
clean-test:
	rm -f test
