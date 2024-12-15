#include "interpreter.h"

static const char* input;
static int         position;
static Token       currentToken;

static int variables[26];

static int executeFlag = 1; 

static Token getToken() {
    Token t;
    while (input[position] && isspace((unsigned char)input[position])) {
        position++;
    }

    if (!input[position]) {
        t.type = T_END;
        t.ch   = 0;
        return t;
    }

    char c = input[position++];
    switch (c) {
        case '[': t.type = T_LBRACKET; t.ch = c; return t;
        case ']': t.type = T_RBRACKET; t.ch = c; return t;
        case '{': t.type = T_LBRACE;   t.ch = c; return t;
        case '}': t.type = T_RBRACE;   t.ch = c; return t;
        case '(': t.type = T_LPAREN;   t.ch = c; return t;
        case ')': t.type = T_RPAREN;   t.ch = c; return t;
        case '?': t.type = T_QUESTION; t.ch = c; return t;
        case ':': t.type = T_COLON;    t.ch = c; return t;
        case ';': t.type = T_SEMI;     t.ch = c; return t;
        case '.': t.type = T_DOT;      t.ch = c; return t;
        case '+': t.type = T_PLUS;     t.ch = c; return t;
        case '-': t.type = T_MINUS;    t.ch = c; return t;
        case '*': t.type = T_STAR;     t.ch = c; return t;
        case '/': t.type = T_SLASH;    t.ch = c; return t;
        case '%': t.type = T_MOD;      t.ch = c; return t;
        case '^': t.type = T_CARET;    t.ch = c; return t;
        case '=': t.type = T_ASSIGN;   t.ch = c; return t;
        case '<': t.type = T_LT;       t.ch = c; return t;
        case '>': t.type = T_GT;       t.ch = c; return t;
        default:
            if (islower((unsigned char)c)) {
                t.type = T_ID;
                t.ch   = c;
                return t;
            } else if (isdigit((unsigned char)c)) {
                t.type = T_NUM;
                t.ch   = c;
                return t;
            } else {
                t.type = T_UNKNOWN;
                t.ch   = c;
                return t;
            }
    }
}

static void getNextToken() {
    currentToken = getToken();
}

static void error(const char* msg) {
    fprintf(stderr, "Parser error: %s\n", msg);
    exit(1);
}

static void initTokenBuffer(TokenBuffer* buf) {
    buf->count = 0;
    buf->capacity = 16;
    buf->tokens = (Token*)malloc(sizeof(Token)*buf->capacity);
}

static void freeTokenBuffer(TokenBuffer* buf) {
    if (buf->tokens) {
        free(buf->tokens);
    }
    buf->tokens = NULL;
    buf->count = 0;
    buf->capacity = 0;
}

static void pushToken(TokenBuffer* buf, Token tk) {
    if (buf->count >= buf->capacity) {
        buf->capacity *= 2;
        buf->tokens = (Token*)realloc(buf->tokens, sizeof(Token)*buf->capacity);
    }
    buf->tokens[buf->count++] = tk;
}

static void ml_init(MiniLexer* ml, Token* tokens, int size) {
    ml->tokens = tokens;
    ml->pos    = 0;
    ml->size   = size;
}

static Token ml_getToken(MiniLexer* ml) {
    if (ml->pos >= ml->size) {
        Token t;
        t.type = T_END;
        t.ch = 0;
        return t;
    }
    return ml->tokens[ml->pos++];
}
static void P() {
    while (currentToken.type != T_DOT) {
        if (currentToken.type == T_END) {
            error("P: Program sonu gelmeden '.' bekleniyor.");
        }
        C();
    }
    getNextToken();
    printf("Program basariyla parse edildi.\n");
}

static void C() {
    switch (currentToken.type) {
        case T_LBRACKET:
            getNextToken();
            I();
            break;
        case T_LBRACE:
            getNextToken();
            W();
            break;
        case T_ID:
            A();
            break;
        case T_LT:
            getNextToken();
            Cik();
            break;
        case T_GT:
            getNextToken();
            Girdi();
            break;
        default:
            error("C: Beklenmeyen token.");
    }
}
static void I() {
    int cond = E(); 

    if (currentToken.type != T_QUESTION) {
        error("IF: '?' bekleniyor.");
    }
    getNextToken(); 

    int savedFlag = executeFlag;
    if (cond != 0) {
        executeFlag = 1;
    } else {
        executeFlag = 0;
    }

    while (currentToken.type != T_COLON && currentToken.type != T_RBRACKET) {
        if (currentToken.type == T_DOT || currentToken.type == T_END) {
            error("IF: ':' veya ']' bekleniyor.");
        }
        C(); 
    }

    if (currentToken.type == T_COLON) {

        getNextToken();
        if (cond != 0) {
            executeFlag = 0;
        } else {
            executeFlag = 1;
        }
        while (currentToken.type != T_RBRACKET) {
            if (currentToken.type == T_DOT || currentToken.type == T_END) {
                error("IF: ']' bekleniyor.");
            }
            C();
        }
    }

    if (currentToken.type != T_RBRACKET) {
        error("IF: ']' bekleniyor.");
    }
    getNextToken();


    executeFlag = savedFlag;
}
static void W() 
{
   
    TokenBuffer condBuf; 
    initTokenBuffer(&condBuf);

    while (currentToken.type != T_QUESTION) {
        if (currentToken.type == T_END || currentToken.type == T_DOT) {
            error("WHILE: Kosul ifadesinde '?' bekleniyor.");
        }
        pushToken(&condBuf, currentToken);
        getNextToken();
    }
    getNextToken();

    TokenBuffer blockBuf;
    initTokenBuffer(&blockBuf);

    while (currentToken.type != T_RBRACE) {
        if (currentToken.type == T_END || currentToken.type == T_DOT) {
            error("WHILE: '}' bekleniyor.");
        }
        pushToken(&blockBuf, currentToken);
        getNextToken();
    }
    getNextToken();

    while (1) {
        MiniLexer mlCond;
        ml_init(&mlCond, condBuf.tokens, condBuf.count);

        int condVal = 0;

       
        int parseE(MiniLexer* ml);

        int parseF(MiniLexer* ml) {
            Token tk = ml_getToken(ml);
            if (tk.type == T_LPAREN) {
                int val = parseE(ml);
                Token tk2 = ml_getToken(ml);
                if (tk2.type != T_RPAREN) {
                    error("WHILE: parantez kapatilmiyor (mini parse).");
                }
                return val;
            } else if (tk.type == T_ID) {
                int idx = tk.ch - 'a';
                return variables[idx];
            } else if (tk.type == T_NUM) {
                return tk.ch - '0';
            } else {
                error("WHILE: parseF beklenmeyen token");
            }
            return 0;
        }

        int parseU(MiniLexer* ml) {
            int left = parseF(ml);

            if (ml->pos < ml->size && ml->tokens[ml->pos].type == T_CARET) {
                ml->pos++;
                int right = parseU(ml);
                int result = 1;
                for(int i=0; i<right; i++){
                    result *= left;
                }
                return result;
            }
            return left;
        }

        int parseT(MiniLexer* ml) {
            int result = parseU(ml);
            while (ml->pos < ml->size) {
                Token op = ml->tokens[ml->pos];
                if (op.type == T_STAR || op.type == T_SLASH || op.type == T_MOD) {
                    ml->pos++;
                    int right = parseU(ml);
                    if (op.type == T_STAR)  result *= right;
                    else if (op.type == T_SLASH) {
                        if (right == 0) error("WHILE: 0'a bolunemez (mini parse).");
                        result /= right;
                    } else if (op.type == T_MOD) {
                        if (right == 0) error("WHILE: 0'a gore mod alinamaz (mini parse).");
                        result %= right;
                    }
                } else {
                    break;
                }
            }
            return result;
        }

        int parseE(MiniLexer* ml) {
            int result = parseT(ml);
            while (ml->pos < ml->size) {
                Token op = ml->tokens[ml->pos];
                if (op.type == T_PLUS || op.type == T_MINUS) {
                    ml->pos++;
                    int right = parseT(ml);
                    if (op.type == T_PLUS)  result += right;
                    else                    result -= right;
                } else {
                    break;
                }
            }
            return result;
        }

        condVal = parseE(&mlCond);

        if (condVal == 0) {
            break;
        }

        int block_E(BlockParser* bp);

        void block_error(const char* msg) { 
            fprintf(stderr, "WHILE block parse error: %s\n", msg);
            exit(1);
        }

        Token block_nextToken(BlockParser* bp) {
            if (bp->pos >= bp->size) {
                Token t;
                t.type = T_END;
                t.ch   = 0;
                return t;
            }
            return bp->tokens[bp->pos++];
        }

        Token block_peekToken(BlockParser* bp) {
            if (bp->pos >= bp->size) {
                Token t;
                t.type = T_END;
                t.ch   = 0;
                return t;
            }
            return bp->tokens[bp->pos];
        }

        void block_C(BlockParser* bp); 
        void block_I(BlockParser* bp);

        void block_parseBlock(BlockParser* bp) {

            while (bp->pos < bp->size) {
                block_C(bp);
            }
        }

        void block_C(BlockParser* bp) {
            Token tk = block_peekToken(bp);
            switch (tk.type) {
                case T_LBRACKET: 
                    block_nextToken(bp);
                    block_I(bp);
                    break;
                case T_LBRACE:
                    block_nextToken(bp); 
                    block_error("Nested while icin extra parser kodu gerekli, ornek iskelet.");
                    break;
                case T_ID: {

                    Token varTok = block_nextToken(bp);
                    Token eqTok  = block_nextToken(bp); 
                    if (eqTok.type != T_ASSIGN) {
                        block_error("WHILE block: Atama icin '=' bekleniyor.");
                    }
                    int val = block_E(bp);
                    Token semi = block_nextToken(bp);
                    if (semi.type != T_SEMI) {
                        block_error("WHILE block: Atama sonunda ';' bekleniyor.");
                    }
                    if (bp->execFlag) {
                        int idx = varTok.ch - 'a';
                        variables[idx] = val;
                    }
                } break;
                case T_LT: {

                    block_nextToken(bp); 
                    int val = block_E(bp);
                    Token semi = block_nextToken(bp);
                    if (semi.type != T_SEMI) {
                        block_error("WHILE block: cıkıs ifadesinde ';' bekleniyor.");
                    }
                    if (bp->execFlag) {
                        printf("%d\n", val);
                    }
                } break;
                case T_GT: {
                    block_nextToken(bp);
                    Token varTok = block_nextToken(bp);
                    if (varTok.type != T_ID) {
                        block_error("WHILE block: Girdi icin ID bekleniyor.");
                    }
                    Token semi = block_nextToken(bp);
                    if (semi.type != T_SEMI) {
                        block_error("WHILE block: Girdi ifadesinde ';' bekleniyor.");
                    }
                    if (bp->execFlag) {
                        int val;
                        printf("Input for variable '%c': ", varTok.ch);
                        scanf("%d", &val);
                        int idx = varTok.ch - 'a';
                        variables[idx] = val;
                    }
                } break;
                default:
                    if (tk.type == T_END) {
                        return;
                    }
                    block_error("WHILE block: Beklenmeyen token C()");
            }
        }

        void block_I(BlockParser* bp) {
            int saved = bp->execFlag;
            int condVal = 0;

            condVal = block_E(bp);

            Token qTok = block_nextToken(bp);
            if (qTok.type != T_QUESTION) {
                block_error("WHILE block: IF icin '?' bekleniyor.");
            }

            if (condVal != 0) {
                bp->execFlag = 1;
            } else {
                bp->execFlag = 0;
            }

            while (1) {
                Token pk = block_peekToken(bp);
                if (pk.type == T_COLON || pk.type == T_RBRACKET || pk.type == T_END) {
                    break;
                }
                block_C(bp);
            }

            pk = block_peekToken(bp);
            if (pk.type == T_COLON) {
                block_nextToken(bp);
                if (condVal != 0) {
                    bp->execFlag = 0;
                } else {
                    bp->execFlag = 1;
                }
                while (1) {
                    Token pk2 = block_peekToken(bp);
                    if (pk2.type == T_RBRACKET || pk2.type == T_END) {
                        break;
                    }
                    block_C(bp);
                }
            }
            Token rb = block_nextToken(bp);
            if (rb.type != T_RBRACKET) {
                block_error("WHILE block: IF icin ']' bekleniyor.");
            }

            bp->execFlag = saved;
        }

        int block_T(BlockParser* bp);

        int block_E(BlockParser* bp) {
            int result = block_T(bp);
            while (1) {
                Token pk = block_peekToken(bp);
                if (pk.type == T_PLUS || pk.type == T_MINUS) {
                    block_nextToken(bp);
                    int right = block_T(bp);
                    if (pk.type == T_PLUS)  result += right;
                    else                    result -= right;
                } else {
                    break;
                }
            }
            return result;
        }

        int block_U(BlockParser* bp);

        int block_T(BlockParser* bp) {
            int result = block_U(bp);
            while (1) {
                Token pk = block_peekToken(bp);
                if (pk.type == T_STAR || pk.type == T_SLASH || pk.type == T_MOD) {
                    block_nextToken(bp);
                    int right = block_U(bp);
                    if (pk.type == T_STAR) {
                        result *= right;
                    } else if (pk.type == T_SLASH) {
                        if (right == 0) {
                            block_error("WHILE block parse: 0'a bolunemez.");
                        }
                        result /= right;
                    } else {
                        if (right == 0) {
                            block_error("WHILE block parse: 0 mod hatasi.");
                        }
                        result %= right;
                    }
                } else {
                    break;
                }
            }
            return result;
        }

        int block_F(BlockParser* bp);

        int block_U(BlockParser* bp) {
            int left = block_F(bp);
            Token pk = block_peekToken(bp);
            if (pk.type == T_CARET) {
                block_nextToken(bp);
                int right = block_U(bp);
                int result = 1;
                for(int i=0; i<right; i++){
                    result *= left;
                }
                return result;
            }
            return left;
        }

        int block_F(BlockParser* bp) {
            Token tk = block_nextToken(bp);
            if (tk.type == T_LPAREN) {
                int val = block_E(bp);
                Token tk2 = block_nextToken(bp);
                if (tk2.type != T_RPAREN) {
                    block_error("WHILE block parse: ')' bekleniyor.");
                }
                return val;
            } else if (tk.type == T_ID) {
                int idx = tk.ch - 'a';
                return variables[idx];
            } else if (tk.type == T_NUM) {
                return tk.ch - '0';
            } else {
                block_error("WHILE block parse: F beklenmeyen token");
            }
            return 0;
        }

        BlockParser bp;
        bp.tokens   = blockBuf.tokens;
        bp.pos      = 0;
        bp.size     = blockBuf.count;
        bp.execFlag = 1;
        block_parseBlock(&bp);
    }

    freeTokenBuffer(&condBuf);
    freeTokenBuffer(&blockBuf);
}

static void A() {
    char varName = currentToken.ch;
    getNextToken();

    if (currentToken.type != T_ASSIGN) {
        error("Atama: '=' bekleniyor.");
    }
    getNextToken();

    int val = E();

    if (currentToken.type != T_SEMI) {
        error("Atama sonunda ';' bekleniyor.");
    }
    getNextToken();

    if (executeFlag) {
        int idx = varName - 'a';
        variables[idx] = val;
    }
}

static void Cik() {
    int val = E();
    if (currentToken.type != T_SEMI) {
        error("Cikti ifadesinde ';' bekleniyor.");
    }
    getNextToken();

    if (executeFlag) {
        printf("%d\n", val);
    }
}

static void Girdi() {
    if (currentToken.type != T_ID) {
        error("Girdi ifadesinde degisken (ID) bekleniyor.");
    }
    char varName = currentToken.ch;
    getNextToken();
    if (currentToken.type != T_SEMI) {
        error("Girdi ifadesinde ';' bekleniyor.");
    }
    getNextToken();

    if (executeFlag) {
        int val;
        printf("Input for variable '%c': ", varName);
        scanf("%d", &val);
        int idx = varName - 'a';
        variables[idx] = val;
    }
}

static int E() {
    int result = T();
    while (currentToken.type == T_PLUS || currentToken.type == T_MINUS) {
        TokenType op = currentToken.type;
        getNextToken();
        int right = T();
        if (op == T_PLUS) result += right;
        else             result -= right;
    }
    return result;
}

static int T() {
    int result = U();
    while (currentToken.type == T_STAR || currentToken.type == T_SLASH || currentToken.type == T_MOD) {
        TokenType op = currentToken.type;
        getNextToken();
        int right = U();
        if (op == T_STAR) {
            result *= right;
        } else if (op == T_SLASH) {
            if (right == 0) {
                error("Bolme hatasi: 0'a bolunemez.");
            }
            result /= right;
        } else if (op == T_MOD) {
            if (right == 0) {
                error("Mod hatasi: 0'a gore mod alinamaz.");
            }
            result %= right;
        }
    }
    return result;
}

static int U() {
    int left = F();
    if (currentToken.type == T_CARET) {
        getNextToken();
        int right = U();
        int result = 1;
        for(int i=0; i<right; i++){
            result *= left;
        }
        return result;
    }
    return left;
}

static int F() {
    if (currentToken.type == T_LPAREN) {
        getNextToken();
        int val = E();
        if (currentToken.type != T_RPAREN) {
            error("Parantez kapatma hatasi: ')' bekleniyor.");
        }
        getNextToken();
        return val;
    } else if (currentToken.type == T_ID) {
        char varName = currentToken.ch;
        getNextToken();
        int idx = varName - 'a';
        return variables[idx];
    } else if (currentToken.type == T_NUM) {
        int val = currentToken.ch - '0';
        getNextToken();
        return val;
    } else {
        error("F: Beklenmeyen token.");
    }
    return 0;
}
