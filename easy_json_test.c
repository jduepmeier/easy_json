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
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* string =  malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;

	return string;
}

int main(int argc, char** argv) {

	ejson_struct* ejson = NULL;

	char* test = strdup("{\"test\":1202}");


	if (argc > 1) {
		free(test);
		test = read_file(argv[1]);
	}
	ejson_parse(&ejson, test);

	print_structure(ejson);

	ejson_cleanup(ejson);
	free(test);

	return 0;
}
