#include "easy_json.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void print_type(ejson_struct* ejson) {

	if (!ejson) {
		return;
	}

	printf("type: ");

	switch(ejson->type) {
		case EJSON_INT:
			printf("int\n");
			break;
		case EJSON_DOUBLE:
			printf("double\n");
			break;
		case EJSON_STRING:
			printf("string\n");
			break;
		case EJSON_OBJECT:
			printf("object\n");
			break;
		case EJSON_ARRAY:
			printf("array\n");
			break;
		case EJSON_BOOLEAN:
			printf("boolean\n");
			break;
		case EJSON_NULL:
			printf("null\n");
			break;
	}
}

void print_structure(ejson_struct* ejson) {

	printf("print structure\n");
	if (ejson) {
		print_type(ejson);
		if (ejson->key) {
			printf("key: (%s)\n", ejson->key);
		}

		if (ejson->value) {
			printf("value: (%s)\n", ejson->value);
		}

		if (ejson->child) {
			printf("-----Next Child------\n");
			print_structure(ejson->child);
		}

		if (ejson->next) {
			printf("----Next Object------\n");
			print_structure(ejson->next);
		}
	} else {
		printf("ejson is null.\n");
	}
}

char* read_file(char* filename) {

	FILE *f = fopen(filename, "rb");

	if (!f) {
		printf("Cannot open file.\n");
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* string =  malloc(fsize + 1);
	if (fread(string, fsize, 1, f) == 0) {
		return NULL;
	}
	fclose(f);

	string[fsize] = 0;

	return string;
}

int main(int argc, char** argv) {

	ejson_struct* ejson = NULL;

	char* test = strdup("{\"test\":1202}");

	if (argc > 1) {

		if (!strcmp(argv[1], "-h")) {
			printf("%s <file>\t validates a json file.\n", argv[0]);
			printf("%s -h\t shows this help.\n", argv[0]);
			return 0;
		}

		free(test);
		test = read_file(argv[1]);
	}

	if (!test) {
		return 1;
	}
	enum ejson_errors error = ejson_parse_warnings(&ejson, test, true, stderr);

	//print_structure(ejson);

	ejson_cleanup(ejson);
	free(test);

	return error;
}
