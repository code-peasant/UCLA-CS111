CC = gcc
CFLAGS = -g
FILES = main.c
NAME = lab1-huiwenz
OUT_EXE = simpsh
TESTS = $(wildcard test*.sh)
TEST_BASES = $(subst .sh,,$(TESTS))
DIST_BASES = main.c Makefile README test.sh

all: $(FILES)
	$(CC) $(CFLAGS) -o $(OUT_EXE) $(FILES)

check:
	@./$(TESTS)

dist:
	@mkdir $(NAME)
	@cp $(DIST_BASES) $(NAME)
	@tar -czf $(NAME).tar.gz $(NAME)
	@rm -fr $(NAME)

clean:
	@rm -fr temp
	@rm $(OUT_EXE) 

.PHONY: all check dist clean
