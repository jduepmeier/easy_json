#include "easy_json.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void print_type(ejson_base* ejson) {

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

extern void print_structure(ejson_base* root);

void print_object(ejson_object* root) {
	int i;
	printf("{\n");
	for (i = 0; i < root->length; i++) {
		printf("%s: ", root->keys[i]->key);
		print_structure(root->keys[i]->value);
		printf("\n");
	}
	printf("}\n");
}

void print_array(ejson_array* root) {

	printf("[\n");
	int i;
	for (i = 0; i < root->length; i++) {
		print_structure(root->values[i]);
		printf("\n");
	}

	printf("]");
}

void print_structure(ejson_base* root) {

	if (root) {
		switch (root->type) {
			case EJSON_INT:
				printf("%ld", ((ejson_number*) root)->value);
				break;
			case EJSON_DOUBLE:
				printf("%f", ((ejson_real*) root)->value);
				break;
			case EJSON_STRING:
				printf("%s", ((ejson_string*) root)->value);
				break;
			case EJSON_BOOLEAN:
				printf("%s", ((ejson_bool*) root)->value ?  "true" : "false");
				break;
			case EJSON_NULL:
				printf("<null>");
				break;
			case EJSON_OBJECT:
				print_object((ejson_object*) root);
				break;
			case EJSON_ARRAY:
				print_array((ejson_array*) root);
				break;
			default:
				printf("<Unkown type>");
				break;
		}
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

	ejson_base* root = NULL;

	char* test = strdup("{\"test\":1202}");

	int i = 1;
	int print = 0;
	if (argc > 1) {

		if (!strcmp(argv[1], "-h")) {
			printf("%s <file>\t validates a json file.\n", argv[0]);
			printf("%s -h\t shows this help.\n", argv[0]);
			printf("%s -p <file>\n", argv[0]);
			return 0;
		} else if (!strcmp(argv[1], "-p")) {
			print = 1;
			i++;
		}
		if (argc > i) {
			free(test);
			test = read_file(argv[i]);
		}
	}

	if (!test) {
		return 1;
	}

	enum ejson_errors error = ejson_parse_warnings(test, strlen(test), true, stderr, &root);

	if (print) {
		print_structure(root);
		printf("\n");
	}

	ejson_cleanup(root);
	free(test);

	return error != EJSON_OK;
}
