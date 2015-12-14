#pragma once

#include <stdbool.h>

enum ejson_errors {
	EJSON_OK = 0,
	EJSON_INVALID_JSON = 1,
	EJSON_WRONG_TYPE = 2
};

enum ejson_types {
	EJSON_INT = 10,
	EJSON_FLOAT = 11,
	EJSON_OBJECT = 12,
	EJSON_STRING = 13,
	EJSON_BOOLEAN = 14,
	EJSON_ARRAY = 15,
	EJSON_NULL = 16
};

typedef struct ejson_struct ejson_struct;
struct ejson_struct {

	enum ejson_types type;
	char* key;
	char* value;
	ejson_struct* child;
	ejson_struct* next;

};

typedef struct {
	enum ejson_errors error;
	char* reason;
	char* pos;
	bool in_array;
	char seperator;
} ejson_state;

enum ejson_errors ejson_get_int(ejson_struct* ejson, int* i);
enum ejson_errors ejson_get_string(ejson_struct* ejson, char* s);
enum ejson_errors ejson_get_boolean(ejson_struct* ejson, bool* b);
enum ejson_errors ejson_parse(ejson_struct** ejson, char* string);
void ejson_cleanup(ejson_struct* ejson);
void ejson_identify(ejson_state* state, ejson_struct** ejson);
