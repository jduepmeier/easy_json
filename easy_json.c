#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "easy_json.h"

ejson_struct* ejson_init_struct() {
	ejson_struct* ejson = malloc(sizeof(ejson_struct));
	ejson->key = NULL;
	ejson->value = NULL;
	ejson->type = -1;
	ejson->child = NULL;
	ejson->next = NULL;

	return ejson;
}

enum ejson_errors ejson_get_int(ejson_struct* ejson, int* i) {


	if (ejson->type != EJSON_INT) {
		return EJSON_WRONG_TYPE;
	}

	int value = strtol(ejson->value, NULL, 10);

	(*i) = value;

	return EJSON_OK;
}

enum ejson_errors ejson_get_string(ejson_struct* ejson, char* s) {

	if (ejson->type != EJSON_STRING) {
		return EJSON_WRONG_TYPE;
	}

	s = ejson->value;

	return EJSON_OK;
}

enum ejson_errors ejson_get_boolean(ejson_struct* ejson, bool* b) {
	if (ejson->type != EJSON_BOOLEAN) {
		return EJSON_WRONG_TYPE;
	}

	switch (*ejson->value) {
		case 't':
			(*b) = true;
			break;
		case 'f':
			(*b) = false;
			break;
		default:
			return EJSON_INVALID_JSON;
	}

	return EJSON_OK;
}

int ejson_check_float(char* s) {

	if (*s == '-' || *s == '+') {
		s++;
	}

	while (*s != 0) {
		if (isdigit(*s)) {
			s++;
		} else {


			switch(*s) {
				case '.':
					return EJSON_DOUBLE;
				case ',':
				case '}':
				case ']':
				case ' ':
					return EJSON_INT;
				default:
					return EJSON_WRONG_TYPE;
			}
		}
	}

	return EJSON_OK;
}

char* ejson_trim(char* string) {
	while (isspace(*string)) {
		string++;
	}

	return string;
}

char* ejson_parse_get_string(ejson_state* state) {

	printf("Get String: (%s)\n", state->pos);
	
	char* string = ejson_trim(state->pos);

	if (*string != '"') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot find leading \".";
		return "";
	}

	string++;
	char* s = string;
	unsigned offset = 0;
	unsigned curr = 0;
	char u[2];

	while (string[curr] != 0) {

		switch (string[curr]) {
			case '\\':
				curr++;
				if (string[curr] == 0) {
					break;
				}
				switch(string[curr]) {
					case '\\':
						string[offset] = '\\';
						break;
					case '"':
						string[offset] = '"';
						break;
					case 'b':
						string[offset] = '\b';
						break;
					case 'f':
						string[offset] = '\f';
						break;
					case 'n':
						string[offset] = '\n';
						break;
					case 'r':
						string[offset] = '\r';
						break;
					case 't':
						string[offset] = '\t';
						break;
					
					// currently no unicode escape support	
					/*
					case 'u':
						curr++;

						if (string[curr] == 0 || string[curr + 1] == 0 || string[curr + 2] == 0 || string[curr + 3] == 0) {
							break;
						}
						strncpy(u, string + curr, 2);
						string[offset] = strtol(u, NULL, 16);
						offset++;
						curr += 2;
						strncpy(u, string + curr, 2);
						string[offset] = strtol(u, NULL, 16);
						offset++;
						curr += 2;
						*/
				}

				break;
			case '"':
				string[curr] = 0;
				string[offset] = 0;
				state->pos = string + curr + 1;
				return s;
			default:
				string[offset] = string[curr];
				break;
		}
		offset++;
		curr++;
	}

	state->error = EJSON_INVALID_JSON;
	state->reason = "Cannot find trailing \".";

	return NULL;
}

void ejson_parse_string(ejson_state* state, ejson_struct** ejson_output) {
	ejson_struct* ejson = *ejson_output;	
	
	char* s = ejson_parse_get_string(state);
	printf("Found string: (%s)\n", s);
	state->pos = ejson_trim(state->pos);
	char* key = NULL;
	
	if (*state->pos == ':') {
		printf("Key found.\n");
		*state->pos = 0;
		state->pos++;
		if (ejson) {
			state->error = EJSON_INVALID_JSON;
			state->reason = "Cannot define more than one key.";
			return;
		}

		key = s;
	}

	if (!ejson) {
		ejson = ejson_init_struct();
		ejson->key = key;
	}

	printf("key is (%s)\n", key ? key : "null");
	if (key) {
		ejson_identify(state, &ejson);
	} else {
		ejson->type = EJSON_STRING;
		ejson->value = s;
	}

	if (state->error != EJSON_OK) {
		printf("state not ok.\n");
		return;
	}
	
	*ejson_output = ejson;

	printf("parse string out.\n");
}

void ejson_parse_array(ejson_state* state, ejson_struct** ejson_output) {
	printf("begin parsing array.\n");
	
	ejson_struct* ejson = *ejson_output;
	
	// check if is a array
	state->pos = ejson_trim(state->pos);
	if (*state->pos != '[') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot find leading [.";
		return;
	}
	ejson_struct* ejson_in_array = NULL;

	// skip [
	state->pos++;

	// create struct if not exists
	if (!ejson) {
		ejson = ejson_init_struct();
		ejson->type = EJSON_ARRAY;
	} else {
		if (ejson->type < 0) {
			ejson->type = EJSON_ARRAY;
		}
	}
		
	ejson_struct* lastChild = NULL;

	// build values
	while (*state->pos != 0 && *state->pos != ']') {
		ejson_identify(state, &ejson_in_array);

		if (state->error != EJSON_OK) {
			return;
		}

		// save in structure
		if (!lastChild) {
			lastChild = ejson_in_array;
			ejson->child = lastChild;
		} else {
			lastChild->next = ejson_in_array;
			lastChild = lastChild->next;
		}

		ejson_in_array = NULL;

		state->pos = ejson_trim(state->pos);

		switch (*state->pos) {
			case ',':
				*state->pos = 0;
				state->pos++;
				break;
			case ']':
				break;
			default:
				printf("current char in parsing array (%c).\n", *state->pos);
				state->error = EJSON_INVALID_JSON;
				state->reason = "Cannot parse this char in array parsing";
				return;
		}
	}

	if (*state->pos != ']') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot find trailing [.";
	}

	*state->pos = 0;
	state->pos++;

	*ejson_output = ejson;
	printf("end parsing array.\n");
}

void ejson_parse_bool(ejson_state* state, ejson_struct** ejson_output) {
	ejson_struct* ejson = *ejson_output;

	printf("parse boolean (%s)\n", state->pos);

	if (!ejson) {
		ejson = ejson_init_struct();
	}
	ejson->type = EJSON_BOOLEAN;

	state->pos = ejson_trim(state->pos);

	if (!strncmp(state->pos, "true", 4)) {
		ejson->value = "true";
		state->pos += 4;
	} else if (!strncmp(state->pos, "false", 5)) {
		ejson->value = "false";
		state->pos += 5;
	} else {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot parse boolean.";
		return;
	}

	*ejson_output = ejson;
}

void ejson_parse_null(ejson_state* state, ejson_struct** ejson_output) {
	ejson_struct* ejson = *ejson_output;
	
	if (!ejson) {
		ejson = malloc(sizeof(ejson_struct));
	}
	
	if (!strncmp(state->pos, "null", 4)) {
		ejson->type = EJSON_NULL;
		ejson->value = NULL;
		state->pos += 4;
	} else {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot parse null.";
	}

	*ejson_output = ejson;
}

void ejson_parse_number(ejson_state* state, ejson_struct** ejson_output) {
	ejson_struct* ejson = *ejson_output;

	printf("Parse number (%s)\n", state->pos);

	if (!ejson) {
		printf("ejson struct is null.\n");
		ejson = malloc(sizeof(ejson_struct));
	}

	ejson->value = state->pos;

	char* end = "";
	strtol(state->pos, &end, 10);

	if (*end == '.') {
		printf("Number is double\n");
		ejson->type = EJSON_DOUBLE;
		strtod(state->pos, &end);
	} else {
		printf("Number is int\n");
		ejson->type = EJSON_INT;
	}
	state->pos = end;
	printf("endptr: %c\n", *state->pos);

	*ejson_output = ejson;
}

void ejson_parse_object(ejson_state* state, ejson_struct** ejson_output) {

	ejson_struct* ejson = *ejson_output;

	printf("Parse object.\n");

	state->pos = ejson_trim(state->pos);

	if (*state->pos != '{') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot find leading {.";
		return;
	}
	*state->pos = 0;
	state->pos++;

	if (!ejson) {
		ejson = ejson_init_struct();
	}

	ejson->type = EJSON_OBJECT;

	ejson_struct* ejson_in_object = NULL;
	ejson_struct* lastChild = NULL;

	printf("Current pos = (%s)\n", state->pos);

	// while there is something
	while (*state->pos != 0 && *state->pos != '}') {
		ejson_identify(state, &ejson_in_object);
		
		// check for error
		if (state->error != EJSON_OK) {
			return;
		}

		// validate key
		if (!ejson_in_object->key) {
			state->error = EJSON_INVALID_JSON;
			state->reason = "Element has no key in object.";
			return;
		}

		if (!lastChild) {
			lastChild = ejson_in_object;
			ejson->child = lastChild; 
		} else {
			lastChild->next = ejson_in_object;
			lastChild = lastChild->next;
		}
		ejson_in_object = NULL;

		state->pos = ejson_trim(state->pos);

		// validate elements
		switch(*state->pos) {
			case ',':
				printf("Found comma.\n");
				*state->pos = 0;
				state->pos++;
				break;
			case '}':
				break;
			default:
				state->error = EJSON_INVALID_JSON;
				state->reason = "Invalid char at this position.";
				printf("char: (%c)\n", *state->pos);
				return;
		}
	}

	if (*state->pos != '}') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot find trailing }.";
		return;
	}

	*state->pos = 0;
	state->pos++;

	*ejson_output = ejson;
	printf("parse object out.\n");
}

void ejson_identify(ejson_state* state, ejson_struct** ejson) {

	printf("Current char (%c)\n", *state->pos);

	state->pos = ejson_trim(state->pos);
	switch(*state->pos) {
		case '"':
			ejson_parse_string(state, ejson);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '-':
		case '+':
			ejson_parse_number(state, ejson);
			break;
		case 't':
		case 'T':
		case 'F':
		case 'f':
			ejson_parse_bool(state, ejson);
			break;
		case '{':
			return ejson_parse_object(state, ejson);
		case '[':
			ejson_parse_array(state, ejson);
			break;
		case 'n':
		case 'N':
			ejson_parse_null(state, ejson);
			break;
		default:
			state->error = EJSON_INVALID_JSON;
			state->reason = "Cannot identify next token.";
			return;
	}
}

void ejson_cleanup(ejson_struct* ejson) {

	if (!ejson) {
		return;
	}

	ejson_cleanup(ejson->child);
	ejson_cleanup(ejson->next);
	free(ejson);
}

enum ejson_errors ejson_parse(ejson_struct** ejson, char* string) {
	ejson_state state = {
		.error = EJSON_OK,
		.reason = "",
		.pos = string
	};
	
	ejson_identify(&state, ejson);

	if (state.error != EJSON_OK) {
		printf("Error: %s\n", state.reason);
	}

	return state.error;
}
