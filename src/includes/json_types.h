#ifndef JSON_TYPES_H
#define JSON_TYPES_H

#include <stdlib.h>

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef  struct JsonValue {
    JsonType type;

    union {
        int boolean;
        double number;
        char *string;

        struct {
            struct JsonValue **items;
            size_t count;
        } array;

        struct {
            char **keys;
            struct JsonValue **values;
        } object;
    } value;
} JsonValue;

typedef struct Json {
    char *key;
    JsonValue *value;
} Json;



#endif
