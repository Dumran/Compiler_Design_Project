#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//---------------------------------------------------------
// 0) Ön tanımlar, veri yapıları
//---------------------------------------------------------

typedef enum {
    T_ID,       // Tek karakter değişken (a-z)
    T_NUM,      // Tek basamak rakam (0-9)
    T_LBRACKET, // '['
    T_RBRACKET, // ']'
    T_LBRACE,   // '{'
    T_RBRACE,   // '}'
    T_LPAREN,   // '('
    T_RPAREN,   // ')'
    T_QUESTION, // '?'
    T_COLON,    // ':'
    T_SEMI,     // ';'
    T_DOT,      // '.'
    T_PLUS,     // '+'
    T_MINUS,    // '-'
    T_STAR,     // '*'
    T_SLASH,    // '/'
    T_MOD,      // '%'
    T_CARET,    // '^'
    T_ASSIGN,   // '='
    T_LT,       // '<'
    T_GT,       // '>'
    T_END,      // Girdi metninin sonu
    T_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char      ch;  // T_ID için hangi harf, T_NUM için hangi rakam vb.
} Token;

// Gramerdeki tüm non-terminal sembolleri parse edecek fonksiyonlarımızın imzaları
static void P();
static void C();
static void I();
static void W();
static void A();
static void Cik();   // "Ç" kuralı için (ekrana çıktı)
static void Girdi(); // "G" kuralı için (kullanıcıdan girdi)
static int  E();
static int  T();
static int  U();
static int  F();

typedef struct {
    Token* tokens;
    int    count;
    int    capacity;
} TokenBuffer;

// parseTokens için mini-lexer state
typedef struct {
    Token* tokens;
    int    pos;
    int    size;
} MiniLexer;

typedef struct {
    Token* tokens;
    int    pos;
    int    size;
    int    execFlag;
} BlockParser;
