.PHONY: run clean afl afl_run

CFLAGS=-Wall -g

all: libeasy_json.a easy_json_test

%.o: %.c easy_json.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

libeasy_json.a: easy_json.o
	${AR} ${ARFLAGS} $@ $>

easy_json_test: easy_json.o easy_json_test.o

run:
	valgrind --leak-check=full ./easy_json_test

clean:
	$(RM) easy_json_test *.o *.a

afl:
	afl-gcc -o afl-easy-json-test -fprofile-arcs -ftest-coverage easy_json.c easy_json_test.c
afl_run:
	afl-fuzz -i tests/ -o afl/afl-output ./afl-easy-json-test @@
