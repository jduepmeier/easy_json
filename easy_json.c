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

void ejson_cleanup(ejson_struct* ejson) {

	if (!ejson) {
		return;
	}

	ejson_cleanup(ejson->child);
	ejson_cleanup(ejson->next);
	free(ejson);
}


ejson_struct* ejson_find_key(ejson_struct* ejson, char* key, bool childs) {

	ejson_struct* ejson_child;
	while (ejson) {

		if (ejson->key && !strcmp(ejson->key, key)) {
			return ejson;
		}

		if (childs && ejson->child) {
			ejson_child = ejson_find_key(ejson->child, key, childs);

			if (ejson_child) {
				return ejson_child;
			}
		}

		ejson = ejson->next;
	}

	return NULL;
}

enum ejson_errors ejson_get_int(ejson_struct* ejson, int* i) {


	if (ejson->type != EJSON_INT) {
		return EJSON_WRONG_TYPE;
	}

	int value = strtol(ejson->value, NULL, 10);

	(*i) = value;

	return EJSON_OK;
}

enum ejson_errors ejson_get_double(ejson_struct* ejson, double* i) {


	if (ejson->type != EJSON_DOUBLE) {
		return EJSON_WRONG_TYPE;
	}

	double value = strtod(ejson->value, NULL);

	(*i) = value;

	return EJSON_OK;
}

enum ejson_errors ejson_get_string(ejson_struct* ejson, char** s) {

	if (ejson->type != EJSON_STRING) {
		return EJSON_WRONG_TYPE;
	}

	*s = ejson->value;

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
	char u[3];
	memset(u, 0, 3);
	unsigned u_1;
	unsigned u_2;

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
					case 'u':
						curr++;
						if (string[curr] == 0 || string[curr + 1] == 0 || string[curr + 2] == 0 || string[curr + 3] == 0) {
							state->error = EJSON_INVALID_JSON;
							state->reason = "Broken unicode.";
							return NULL;
						}
						//printf("Unicode: %.4s\n", string + curr);

						strncpy(u, string + curr, 2);
						u_1 = strtoul(u, NULL, 16);
						curr += 2;
						strncpy(u, string + curr, 2);
						u_2 = strtoul(u, NULL, 16);
						if (u_1 == 0x00 && u_2 <= 0x7F) {
							string[offset] = u_2;
						} else if (u_1 >= 0 && u_1 <= 0x07 && u_2 >= 0x80) {
							string[offset] = 0xC0;
							string[offset] |= (u_1 & 0x07) << 2 | (0xC0 & u_2) >> 6;
							offset++;
							string[offset] = 0x80;
							string[offset] |= u_2 & 0x3F;
						} else if (u_1 >= 0x80) {
							string[offset] = 0xE0;
							string[offset] |= (u_1 & 0xF0) >> 4;
							offset++;
							string[offset] = 0x80;
							string[offset] |= (u_1 & 0x0F) << 2 | (u_2 & 0xC0) >> 6;
							offset++;
							string[offset] = 0x80;
							string[offset] |= u_2 & 0x3F;
						}
						curr++;
						break;
					default:
						state->pos = string + curr;
						state->error = EJSON_INVALID_JSON;
						state->reason = "Unkown escape character.";
						return NULL;
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

ejson_struct* ejson_parse_string(ejson_state* state, ejson_struct* origin) {
	char* s = ejson_parse_get_string(state);
	if (state->error != EJSON_OK) {
		ejson_cleanup(origin);
		return NULL;
	}

	state->pos = ejson_trim(state->pos);
	char* key = NULL;

	if (*state->pos == ':') {
		*state->pos = 0;
		state->pos++;
		if (origin) {
			state->error = EJSON_INVALID_JSON;
			state->reason = "Cannot define more than one key.";
			ejson_cleanup(origin);
			return NULL;
		}

		key = s;
	}

	if (!origin) {
		origin = ejson_init_struct();
		origin->key = key;
	}

	if (key) {
		origin = ejson_identify(state, origin);
	} else {
		origin->type = EJSON_STRING;
		origin->value = s;
	}

	if (state->error != EJSON_OK) {
		ejson_cleanup(origin);
		return NULL;
	}

	return origin;
}

ejson_struct* ejson_parse_array(ejson_state* state, ejson_struct* origin) {

	// check if is a array
	state->pos = ejson_trim(state->pos);
	if (*state->pos != '[') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot find leading [.";
		ejson_cleanup(origin);
		return NULL;
	}

	// skip [
	state->pos++;

	// create struct if not exists
	if (!origin) {
		origin = ejson_init_struct();
	}

	origin->type = EJSON_ARRAY;

	ejson_struct* ejson_in_array = NULL;
	ejson_struct* lastChild = NULL;

	// build values
	while (*state->pos != 0 && *state->pos != ']') {
		ejson_in_array = ejson_identify(state, NULL);

		if (state->error != EJSON_OK) {
			ejson_cleanup(origin);
			return NULL;
		}

		// save in structure
		if (!lastChild) {
			lastChild = ejson_in_array;
			origin->child = lastChild;
		} else {
			lastChild->next = ejson_in_array;
			lastChild = lastChild->next;
		}

		if (ejson_in_array->key) {
			state->error = EJSON_INVALID_JSON;
			state->reason = "No key allowed in json array.";
			ejson_cleanup(origin);
			return NULL;
		}

		ejson_in_array = NULL;

		state->pos = ejson_trim(state->pos);

		switch (*state->pos) {
			case ',':
				*state->pos = 0;
				state->pos++;
				if (*state->pos == ']') {
					state->error = EJSON_INVALID_JSON;
					state->reason = "Trailing comma is not allowed in array.";
					ejson_cleanup(origin);
					return NULL;
				}
				break;
			case ']':
				break;
			default:
				state->error = EJSON_INVALID_JSON;
				state->reason = "Cannot parse this char in array parsing";
				ejson_cleanup(origin);
				return NULL;
		}
	}

	if (*state->pos != ']') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot find trailing [.";
		ejson_cleanup(origin);
		return NULL;
	}

	*state->pos = 0;
	state->pos++;
	return origin;
}

ejson_struct* ejson_parse_bool(ejson_state* state, ejson_struct* origin) {
	if (!origin) {
		origin = ejson_init_struct();
	}
	origin->type = EJSON_BOOLEAN;

	state->pos = ejson_trim(state->pos);

	if (!strncmp(state->pos, "true", 4)) {
		origin->value = "true";
		state->pos += 4;
	} else if (!strncmp(state->pos, "false", 5)) {
		origin->value = "false";
		state->pos += 5;
	} else {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot parse boolean.";
		ejson_cleanup(origin);
		return NULL;
	}
	return origin;
}

ejson_struct* ejson_parse_null(ejson_state* state, ejson_struct* origin) {
	if (!origin) {
		origin = ejson_init_struct();
	}

	if (!strncmp(state->pos, "null", 4)) {
		origin->type = EJSON_NULL;
		origin->value = NULL;
		state->pos += 4;
	} else {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot parse null.";
		ejson_cleanup(origin);
		return NULL;
	}
	return origin;
}

ejson_struct* ejson_parse_number(ejson_state* state, ejson_struct* origin) {
	char* leading_test = state->pos;
	if (*leading_test == '-') {
		leading_test++;
	}

	// check for leading zeros
	if (leading_test[0] != 0 && leading_test[0] == '0' && leading_test[1] != '.') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "invalid number.";
		ejson_cleanup(origin);
		return NULL;
	}

	if (!origin) {
		origin = ejson_init_struct();
	}

	origin->value = state->pos;

	char* end = "";
	strtol(state->pos, &end, 10);
	if (*end == '.') {
		origin->type = EJSON_DOUBLE;
		end = "";
	} else {
		origin->type = EJSON_INT;
	}

	if (state->pos == end) {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot parse number.";
		ejson_cleanup(origin);
		return NULL;
	}
	state->pos = end;
	return origin;
}

ejson_struct* ejson_parse_object(ejson_state* state, ejson_struct* origin) {

	ejson_struct* ejson = origin;
	state->pos = ejson_trim(state->pos);

	if (*state->pos != '{') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot find leading {.";
		ejson_cleanup(ejson);
		return NULL;
	}
	*state->pos = 0;
	state->pos++;

	if (!ejson) {
		ejson = ejson_init_struct();
	}

	ejson->type = EJSON_OBJECT;

	ejson_struct* ejson_in_object = NULL;
	ejson_struct* lastChild = NULL;

	// while there is something
	while (*state->pos != 0 && *state->pos != '}') {
		ejson_in_object = ejson_identify(state, NULL);

		// check for error
		if (state->error != EJSON_OK) {
			ejson_cleanup(ejson);
			return NULL;
		}

		// validate key
		if (!ejson_in_object->key) {
			state->error = EJSON_INVALID_JSON;
			state->reason = "Element has no key in object.";
			ejson_cleanup(ejson_in_object);
			ejson_cleanup(ejson);
			return NULL;
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
				*state->pos = 0;
				state->pos++;
				break;
			case '}':
				break;
			default:
				state->error = EJSON_INVALID_JSON;
				state->reason = "Invalid char at this position.";
				ejson_cleanup(ejson);
				return NULL;
		}
	}

	if (*state->pos != '}') {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Cannot find trailing }.";
		ejson_cleanup(ejson);
		return NULL;
	}

	*state->pos = 0;
	state->pos++;
	return ejson;
}

ejson_struct* ejson_identify(ejson_state* state, ejson_struct* origin) {

	state->counter++;

	if (state->counter > 1000) {
		state->error = EJSON_INVALID_JSON;
		state->reason = "Too many objects (Max size is 1000).";
		ejson_cleanup(origin);
		return NULL;
	}

	state->pos = ejson_trim(state->pos);
	switch(*state->pos) {
		case '"':
			origin = ejson_parse_string(state, origin);
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
			//printf("parse number with starting: %.5s\n", state->pos);
			origin = ejson_parse_number(state, origin);
			break;
		case 't':
		case 'T':
		case 'F':
		case 'f':
			origin = ejson_parse_bool(state, origin);
			break;
		case '{':
			origin = ejson_parse_object(state, origin);
			break;
		case '[':
			origin = ejson_parse_array(state, origin);
			break;
		case 'n':
		case 'N':
			origin = ejson_parse_null(state, origin);
			break;
		default:
			state->error = EJSON_INVALID_JSON;
			state->reason = "Cannot identify next token. Unkown identifier";
			ejson_cleanup(origin);
			return NULL;
	}

	return origin;
}

enum ejson_errors ejson_parse(ejson_struct** ejson, char* string) {

	return ejson_parse_warnings(ejson, string, false, stderr);
}

enum ejson_errors ejson_parse_warnings(ejson_struct** ejson, char* string, bool warnings, FILE* log) {
	ejson_state state = {
		.error = EJSON_OK,
		.reason = "",
		.counter = 0l,
		.pos = string,
		.warnings = warnings,
		.log = log
	};

	if (!state.log) {
		state.log = stderr;
	}

	*ejson = ejson_identify(&state, *ejson);

	if (state.error == EJSON_OK) {

		state.pos = ejson_trim(state.pos);

		if (strlen(state.pos) != 0) {
			state.error = EJSON_INVALID_JSON;
			state.reason = "There are characters after the structure.";
		}
	}

	if (state.error != EJSON_OK) {
		ejson_cleanup(*ejson);
		*ejson = NULL;
	}
	if (state.error != EJSON_OK && state.warnings) {

		int p = (state.pos - string);
		fprintf(state.log, "Error: %s (%d: %c).\n", state.reason, p, *state.pos);
	}

	return state.error;
}
