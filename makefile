.PHONY: run clean
CFLAGS=-Wall -g

easy_json_test: easy_json.c easy_json_test.c

run:
	valgrind --leak-check=full ./easy_json_test

clean:
	$(RM) easy_json_test
