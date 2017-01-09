.PHONY: run clean
CFLAGS=-Wall -g

all: easy_json_test easy_json_test_debug

easy_json_test: easy_json.c easy_json_test.c

easy_json_test_debug: easy_json.c easy_json_test_debug.c

run:
	valgrind --leak-check=full ./easy_json_test

clean:
	$(RM) easy_json_test

afl:
	afl-gcc -o afl-easy-json-test -fprofile-arcs -ftest-coverage easy_json.c easy_json_test.c
afl_run:
	afl-fuzz -i tests/ -o afl-output ./afl-easy-json-test @@
