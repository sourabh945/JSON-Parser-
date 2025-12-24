#include <ctype.h>
#include <endian.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_STRING,
  TOKEN_NUMBER,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_NULL,
  TOKEN_COLON,
  TOKEN_COMMA,
  TOKEN_EOF,
  TOKEN_LEFT_BRACKET,
  TOKEN_RIGHT_BRACKET
} TokenType;

typedef struct {
  TokenType type;
  const char *value;
} Token;

typedef struct {
  const char *input;
  size_t pos;
  size_t length;
  size_t line;
  size_t column;
} Lexer;

// Defining the token vector
//
typedef struct TokenVector {
  Token *tokens;
  size_t count;
  size_t capacity;
} TokenVector;

typedef enum {
  LEX_OK,
  LEX_ERROR_UNTERMINATED_STRING,
  LEX_ERROR_INVALID_ESCAPE,
  LEX_ERROR_INVALID_NUMBER,
  LEX_ERROR_INVALID_KEYWORD,
  LEX_ERROR_UNEXPECTED_CHAR,
  LEX_ERROR_MEMORY
} LexErrorType;

typedef struct {
  LexErrorType type;
  size_t position;      // Character position in input
  size_t line;          // Line number
  size_t column;        // Column number
  char unexpected_char; // The character that caused the error
  char expected[64];    // What was expected
  char context[100];    // Surrounding context for debugging
} LexError;

typedef struct {
  bool success;
  TokenVector *tokens;
  LexError error;
} LexResult;

// create a lexer
Lexer *lexer_create(const char *input) {
  Lexer *lex = malloc(sizeof(Lexer));
  lex->input = input;
  lex->pos = 0;
  lex->length = strlen(input);
  lex->line = 1;
  lex->column = 1;
  return lex;
}

// Peek at current character without consuming it
char lexer_peak(Lexer *lex) {
  if (lex->pos >= lex->length)
    return '\0';
  return lex->input[lex->pos];
}

// Consume current character and advance
char lexer_advance(Lexer *lex) {
  if (lex->pos >= lex->length)
    return '\0';
  lex->column++;
  return lex->input[lex->pos++];
}

// Skip Whitespace
void lexer_skip_whitespace(Lexer *lex) {
  while (lex->pos < lex->length) {
    char c = lex->input[lex->pos];
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      lex->pos++;
      if (c == '\n') {
        lex->line++;
        lex->column = 1;
      } else if (c == '\t')
        lex->column += 4;
      else
        lex->column++;
    } else
      break;
  }
}

LexError create_error(Lexer *lex, LexErrorType type, const char *expected) {
  LexError err = {0};
  err.type = type;
  err.position = lex->pos;
  err.line = lex->line;
  err.column = lex->column;
  err.unexpected_char = lexer_peak(lex);

  if (expected)
    strncpy(err.expected, expected, sizeof(err.expected) - 1);

  // Extract context (50 chars before and after error position)
  size_t start = lex->pos > 50 ? lex->pos - 50 : 0;
  size_t end = lex->pos + 50;
  if (end > lex->length)
    end = lex->length;

  size_t context_len = end - start;
  if (context_len >= sizeof(err.context)) {
    context_len = sizeof(err.context) - 1;
  }

  strncpy(err.context, &lex->input[start], context_len);
  err.context[context_len] = '\0';

  return err;
}

Token *create_token(TokenType type, const char *value) {
  Token *token = malloc(sizeof(Token));
  token->type = type;
  token->value = value;
  return token;
}

// Get the string token from the lexer
Token *lexer_scan_string(Lexer *lex) {
  lexer_advance(lex); // Skip Opening
  size_t start = lex->pos;
  while (lexer_peak(lex) != '"' && lexer_peak(lex) != '\0') {
    lexer_advance(lex);
  }
  size_t length = lex->pos - start;
  char *str = malloc(length + 1);
  if (str == NULL)
    return NULL;
  strncpy(str, &lex->input[start], length);
  str[length] = '\0';

  lexer_advance(lex); // Skip Closing

  return create_token(TOKEN_STRING, str);
}

// Note: Supporting Scientific numbers also
bool is_numeric_char(char c) {
  return isdigit((unsigned char)c) || c == '.' || c == '+' || c == '-' ||
         c == 'E' || c == 'e';
}

// Get the number tokens from the lexer as strings.
// Note: convert the string number to the actual numbers and it support
// scientific number to
Token *lexer_scan_number(Lexer *lex) {
  size_t start = lex->pos;
  while (lexer_peak(lex) != '\0' && is_numeric_char(lexer_peak(lex))) {
    lexer_advance(lex);
  }
  size_t length = lex->pos - start;
  char *str = malloc(length + 1);
  if (str == NULL)
    return NULL;
  strncpy(str, &lex->input[start], length);
  str[length] = '\0';

  return create_token(TOKEN_NUMBER, str);
}

// Get the keywords from the lexer (null, true, false)
Token *lexer_scan_keyword(Lexer *lex, const char start_char) {
  char true_str[] = {'r', 'u', 'e'};
  char false_str[] = {'a', 'l', 's', 'e'};
  char null_str[] = {'u', 'l', 'l'};

  if (start_char == 't') {
    for (int i = 0; i < 3; i++) {
      if (lexer_peak(lex) != true_str[i]) {
        return NULL;
      }
      lexer_advance(lex);
    }
    return create_token(TOKEN_TRUE, NULL);
  } else if (start_char == 'f') {
    for (int i = 0; i < 4; i++) {
      if (lexer_peak(lex) != false_str[i]) {
        return NULL;
      }
      lexer_advance(lex);
    }
    return create_token(TOKEN_FALSE, NULL);
  } else if (start_char == 'n') {
    for (int i = 0; i < 3; i++) {
      if (lexer_peak(lex) != null_str[i]) {
        return NULL;
      }
      lexer_advance(lex);
    }
    return create_token(TOKEN_NULL, NULL);
  }

  return NULL;
}

// Get the next Token
Token *lexer_next_token(Lexer *lex) {
  lexer_skip_whitespace(lex);

  char c = lexer_advance(lex);

  switch (c) {
  case '\0':
    return create_token(TOKEN_EOF, NULL);
  case '{':
    return create_token(TOKEN_LEFT_BRACE, NULL);
  case '}':
    return create_token(TOKEN_RIGHT_BRACE, NULL);
  case '[':
    return create_token(TOKEN_LEFT_BRACKET, NULL);
  case ']':
    return create_token(TOKEN_RIGHT_BRACKET, NULL);
  case ':':
    return create_token(TOKEN_COLON, NULL);
  case ',':
    return create_token(TOKEN_COMMA, NULL);
  case '"':
    return lexer_scan_string(lex);
  case 't':
  case 'f':
  case 'n':
    return lexer_scan_keyword(lex, c);
  default:
    if (c == '-' || isdigit(c)) {
      lex->pos--; // Go back one
      return lexer_scan_number(lex);
    }
  }

  return NULL; // Error: unexpected character
}

// create the vector
TokenVector *create_token_vector() {
  TokenVector *vector = malloc(sizeof(TokenVector));
  vector->tokens = malloc(sizeof(Token) * 10);
  vector->count = 0;
  vector->capacity = 10;
  return vector;
}

// add tokens to vector
void add_token(TokenVector *vector, Token *token) {
  if (vector->count >= vector->capacity) {
    // Double the capacity
    vector->capacity *= 2;
    vector->tokens = realloc(vector->tokens, sizeof(Token) * vector->capacity);
  }
  vector->tokens[vector->count++] = *token;
}

// Get the token from the vector using index
Token get_token(TokenVector *vector, size_t index) {
  return vector->tokens[index];
}

// Getting the size of the vector
size_t length_vector(TokenVector *vector) { return vector->count; }

void free_token(Token *token) {
  if (token->type != TOKEN_STRING && token->type != TOKEN_NUMBER)
    return;
  if (token->value == NULL)
    return;

  free((void *)token->value);
  token->value = NULL;
}

// Free the vector
void free_token_vector(TokenVector *vector) {
  for (size_t i = 0; i < vector->count; i++) {
    free_token(&vector->tokens[i]);
  }
  free(vector->tokens);
  free(vector);
}

TokenVector *tokenize(const char *input) {
  Lexer *lex = lexer_create(input);
  if (!lex)
    return NULL;

  TokenVector *tokens = create_token_vector();
  if (!tokens) {
    free(lex);
    return NULL;
  }

  Token *token;
  while (true) {
    token = lexer_next_token(lex);

    if (token == NULL) {
      // Error occurred
      free_token_vector(tokens);
      free(lex);
      return NULL;
    }

    if (token->type == TOKEN_EOF) {
      free(token);
      break;
    }

    add_token(tokens, token);
    free(token); // Free the struct, not the value (already copied)
  }

  free(lex);
  return tokens;
}
