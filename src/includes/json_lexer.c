#include <ctype.h>
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
  TOKEN_QUOTE,
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
} Lexer;

// create a lexer
Lexer *lexer_create(const char *input) {
  Lexer *lex = malloc(sizeof(Lexer));
  lex->input = input;
  lex->pos = 0;
  lex->length = strlen(input);
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
  return lex->input[lex->pos++];
}

// Skip Whitespace
void lexer_skip_whitespace(Lexer *lex) {
  while (lex->pos < lex->length) {
    char c = lex->input[lex->pos];
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
      lex->pos++;
    else
      break;
  }
}

Token *create_token(TokenType type, const char *value) {
  Token *token = malloc(sizeof(Token));
  token->type = type;
  token->value = value;
  return token;
}

// Get the string token from the lexer
Token *lexer_scan_string(Lexer *lex) {
  size_t start = lex->pos;
  while (lexer_peak(lex) != '"' && lexer_peak(lex) != '\0') {
    lexer_advance(lex);
  }
  size_t length = lex->pos - start;
  char *str = malloc(length + 1);
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
// Note: convert the string number to the actual numbers and it support scientific number to
Token *lexer_scan_number(Lexer *lex) {
  size_t start = lex->pos;
  while (lexer_peak(lex) != '\0' && is_numeric_char(lexer_peak(lex))) {
    lexer_advance(lex);
  }
  size_t length = lex->pos - start;
  char *str = malloc(length + 1);
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
    }
    else if (start_char == 'f') {
        for (int i = 0; i < 4; i++) {
            if (lexer_peak(lex) != false_str[i]) {
                return NULL;
            }
            lexer_advance(lex);
        }
        return create_token(TOKEN_FALSE, NULL);
    }
    else if (start_char == 'n') {
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
        case '\0': return create_token(TOKEN_EOF, NULL);
        case '{':  return create_token(TOKEN_LEFT_BRACE, NULL);
        case '}':  return create_token(TOKEN_RIGHT_BRACE, NULL);
        case '[':  return create_token(TOKEN_LEFT_BRACKET, NULL);
        case ']':  return create_token(TOKEN_RIGHT_BRACKET, NULL);
        case '"':  return lexer_scan_string(lex);
        case 't':
        case 'f':
        case 'n':  return lexer_scan_keyword(lex, c);
        default:
            if (c == '-' || isdigit(c)) {
                lex->pos--;  // Go back one
                return lexer_scan_number(lex);
            }
    }

    return NULL;  // Error: unexpected character
}
