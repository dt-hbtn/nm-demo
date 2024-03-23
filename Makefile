CC := gcc
CFLAGS := -Wall -Werror -Wextra -pedantic
TEST_EXE := test-symbolreader
SRC_FILES := $(wildcard *.c)
OBJ_FILES := $(SRC_FILES:.c=.o)

.PHONY := all clean

all: $(TEST_EXE)

$(TEST_EXE): $(OBJ_FILES)
	@$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f *.o $(TEST_EXE)
