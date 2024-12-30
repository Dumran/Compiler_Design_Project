#ifndef INTERPRETER_H
# define INTERPRETER_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <ctype.h>
# include <stddef.h>

typedef enum
{
    T_ID,
    T_NUM,
    T_LBRACKET,
    T_RBRACKET,
    T_LBRACE,
    T_RBRACE,
    T_LPAREN,
    T_RPAREN,
    T_QUESTION,
    T_COLON,
    T_SEMI,
    T_DOT,
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_MOD,
    T_CARET,
    T_ASSIGN,
    T_LT,
    T_GT,
    T_END,
    T_UNKNOWN
} TokenType;

typedef struct
{
    TokenType type;
    char      ch;  
} Token;

typedef struct
{
    Token* tokens;
    int    pos;
    int    size;
} MiniLexer;

typedef struct
{
    Token* tokens;
    int    pos;
    int    size;
    int    execFlag;
} BlockParser;

typedef struct
{
    Token* tokens;
    int    count;
    int    capacity;
} TokenBuffer;

int	ft_isalpha(int c);
int	ft_isdigit(int c);
int ft_isspace(char c);
void *ft_memset(void *b, int c, size_t len);

void interpret(const char* programText);

#endif