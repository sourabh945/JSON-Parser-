#ifndef JSON_TYPES_H
#define JSON_TYPES_H

#include <stdlib.h>
#include <stdbool.h>

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
        bool boolean;
        double number;
        char *string;

        struct {
            struct JsonValue **items;
            size_t count;
        } array;

        struct {
            char **keys;
            struct JsonValue **values;
            size_t count;
        } object;
    } value;
} JsonValue;

#endif
