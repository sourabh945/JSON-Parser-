#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_STRING,
  TOKEN_INT_NUMBER,
  TOKEN_FLOAT_NUMBER,
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
    if (lex->pos >= lex->length) return '\0';
    return lex->input[lex->pos];
}

// Consume current character and advance
char lexer_advance(Lexer *lex) {
    if (lex->pos >= lex->length) return '\0';
    return lex->input[lex->pos++];
}

// Skip Whitespace
void lexer_skip_whitespace(Lexer *lex) {
    while(lex->pos < lex->length) {
        char c = lex->input[lex->pos];
        if(c == ' ' || c == '\t' || c == '\n' || c == '\r')
            lex->pos++;
        else
            break;
    }
}

Token* create_token(TokenType type, const char *value){
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    return token;
}

// Get the string token from the lexer
Token* lexer_scan_string(Lexer *lex) {
    size_t start = lex->pos;
    while(lexer_peak(lex) != '"' && lexer_peak(lex) != '\0'){
        lexer_advance(lex);
    }
    size_t length = lex->pos - start;
    char *str = malloc(length + 1);
    strncpy(str, &lex->input[start], length);
    str[length] = '\0';

    lexer_advance(lex); // Skip Closing

    return create_token(TOKEN_STRING, str);
}
