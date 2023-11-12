SRC := src/main.c

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
