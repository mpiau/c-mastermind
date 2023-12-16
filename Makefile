SRC := src/main.c
SRC += src/mastermind.c
SRC += src/game_menus.c
SRC += src/keyboard_inputs.c
SRC += src/menu_stack.c
SRC += src/widgets/widget.c
SRC += src/console.c
SRC += src/fps_counter.c

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
