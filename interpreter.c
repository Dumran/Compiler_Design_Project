#include "interpreter.h"

static const char* inputText;
static int         position;
static Token       currentToken;
static int         variables[26];
static int         executeFlag = 1;

void interpret(const char* programText);

static Token getToken(void);
static void  getNextToken(void);
static void  reportError(const char* msg);

static void parseP(void);
static void parseC(void);
static void parseIf(void);
static void parseWhile(void);
static void parseAssignment(void);
static void parseOutput(void);
static void parseInput(void);

static int  parseExpr(void);
static int  parseTerm(void);
static int  parsePower(void);
static int  parseFactor(void);

static int miniParseE(MiniLexer* ml);
static int miniParseT(MiniLexer* ml);
static int miniParseU(MiniLexer* ml);
static int miniParseF(MiniLexer* ml);

static void   blockParse(BlockParser* bp);
static void   blockStatement(BlockParser* bp);
static void   blockIf(BlockParser* bp);
static int    blockExpr(BlockParser* bp);
static int    blockTerm(BlockParser* bp);
static int    blockPower(BlockParser* bp);
static int    blockFactor(BlockParser* bp);
static Token  blockNextToken(BlockParser* bp);
static Token  blockPeekToken(BlockParser* bp);
static void   blockError(const char* msg);

static void initTokenBuffer(TokenBuffer* buf);
static void freeTokenBuffer(TokenBuffer* buf);
static void pushToken(TokenBuffer* buf, Token tk);

void interpret(const char* programText)
{
    inputText  = programText;
    position   = 0;
    ft_memset(variables, 0, sizeof(variables));
    getNextToken();
    parseP();
}

static Token getToken(void)
{
    Token t;
    while (inputText[position] && ft_isspace((unsigned char)inputText[position]))
        position++;

    if (!inputText[position])
    {
        t.type = T_END;
        t.ch   = 0;
        return t;
    }

    char c = inputText[position++];
    switch (c)
    {
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
            if (ft_isalpha((unsigned char)c))
            {
                t.type = T_ID;
                t.ch   = c;
            }
            else if (ft_isdigit((unsigned char)c))
            {
                t.type = T_NUM;
                t.ch   = c;
            }
            else
            {
                t.type = T_UNKNOWN;
                t.ch   = c;
            }
            return t;
    }
}
static void getNextToken(void)
{
    currentToken = getToken();
}

static void reportError(const char* msg)
{
    fprintf(stderr, "Parser Error: %s\n", msg);
    exit(1);
}

static void parseP(void)
{
    while (currentToken.type != T_DOT)
    {
        if (currentToken.type == T_END)
            reportError("Expected '.' before end of program");
        parseC();
    }
    getNextToken();
    printf("Program successfully parsed.\n");
    fflush(stdout);
}

static void parseC(void)
{
    switch (currentToken.type)
    {
        case T_LBRACKET:
            getNextToken();
            parseIf();
            break;

        case T_LBRACE:
            getNextToken();
            parseWhile();
            break;

        case T_ID:
            parseAssignment();
            break;

        case T_LT:
            getNextToken();
            parseOutput();
            break;

        case T_GT:
            getNextToken();
            parseInput();
            break;

        default:
            reportError("Unexpected token in parseC");
    }
}

static void parseIf(void)
{
    int condition = parseExpr();

    if (currentToken.type != T_QUESTION)
        reportError("Missing '?' in IF statement");
    getNextToken();

    int savedExec = executeFlag;
    if (condition != 0)
        executeFlag = 1;
    else
        executeFlag = 0;

    while (currentToken.type != T_COLON && currentToken.type != T_RBRACKET)
    {
        if (currentToken.type == T_DOT || currentToken.type == T_END)
            reportError("Missing ':' or ']' in IF");
        parseC();
    }

    if (currentToken.type == T_COLON)
    {
        getNextToken();
        if (condition != 0)
            executeFlag = 0;
        else
            executeFlag = 1;

        while (currentToken.type != T_RBRACKET)
        {
            if (currentToken.type == T_DOT || currentToken.type == T_END)
                reportError("Missing ']' in IF");
            parseC();
        }
    }

    if (currentToken.type != T_RBRACKET)
        reportError("Missing ']' in IF");

    getNextToken();
    executeFlag = savedExec;
}

static void parseWhile(void)
{
    TokenBuffer condBuf;
    initTokenBuffer(&condBuf);

    while (currentToken.type != T_QUESTION)
    {
        if (currentToken.type == T_END || currentToken.type == T_DOT)
            reportError("Missing '?' in WHILE condition");
        pushToken(&condBuf, currentToken);
        getNextToken();
    }
    getNextToken();
    TokenBuffer blockBuf;
    initTokenBuffer(&blockBuf);

    while (currentToken.type != T_RBRACE)
    {
        if (currentToken.type == T_END || currentToken.type == T_DOT)
            reportError("Missing '}' in WHILE block");
        pushToken(&blockBuf, currentToken);
        getNextToken();
    }
    getNextToken();

    while (1)
    {
        MiniLexer mlCond;
        mlCond.tokens = condBuf.tokens;
        mlCond.pos    = 0;
        mlCond.size   = condBuf.count;

        int condVal = miniParseE(&mlCond);
        if (condVal == 0)
            break;

        BlockParser bp;
        bp.tokens   = blockBuf.tokens;
        bp.pos      = 0;
        bp.size     = blockBuf.count;
        bp.execFlag = 1;

        blockParse(&bp);
    }

    freeTokenBuffer(&condBuf);
    freeTokenBuffer(&blockBuf);
}

static void parseAssignment(void)
{
    char varName = currentToken.ch;
    getNextToken();

    if (currentToken.type != T_ASSIGN)
        reportError("Missing '=' in assignment");
    getNextToken();

    int val = parseExpr();

    if (currentToken.type != T_SEMI)
        reportError("Missing ';' at the end of assignment");
    getNextToken();

    if (executeFlag)
    {
        int idx = varName - 'a';
        variables[idx] = val;
    }
}

static void parseOutput(void)
{
    int val = parseExpr();
    if (currentToken.type != T_SEMI)
        reportError("Missing ';' after output expression");
    getNextToken();

    if (executeFlag)
    {
        printf("%d\n", val);
        fflush(stdout);
    }
}

static void parseInput(void)
{
    if (currentToken.type != T_ID)
        reportError("Missing variable ID in input statement");

    char varName = currentToken.ch;
    getNextToken();

    if (currentToken.type != T_SEMI)
        reportError("Missing ';' after input statement");
    getNextToken();

    if (executeFlag)
    {
        int val;
        printf("Input for variable '%c': ", varName);
        fflush(stdout);
        scanf("%d", &val);
        int idx = varName - 'a';
        variables[idx] = val;
    }
}

static int parseExpr(void)
{
    int result = parseTerm();
    while (currentToken.type == T_PLUS || currentToken.type == T_MINUS)
    {
        TokenType op = currentToken.type;
        getNextToken();
        int right = parseTerm();
        if (op == T_PLUS)  result += right;
        else               result -= right;
    }
    return result;
}

static int parseTerm(void)
{
    int result = parsePower();
    while (currentToken.type == T_STAR || currentToken.type == T_SLASH || currentToken.type == T_MOD)
    {
        TokenType op = currentToken.type;
        getNextToken();
        int right = parsePower();
        if (op == T_STAR)
            result *= right;
        else if (op == T_SLASH)
        {
            if (right == 0)
                reportError("Division by zero");
            result /= right;
        }
        else if (op == T_MOD)
        {
            if (right == 0)
                reportError("Modulo by zero");
            result %= right;
        }
    }
    return result;
}

static int parsePower(void)
{
    int left = parseFactor();
    if (currentToken.type == T_CARET)
    {
        getNextToken();
        int right = parsePower();
        int out = 1;
        for(int i = 0; i < right; i++)
            out *= left;
        return out;
    }
    return left;
}

static int parseFactor(void)
{
    if (currentToken.type == T_LPAREN)
    {
        getNextToken();
        int val = parseExpr();
        if (currentToken.type != T_RPAREN)
            reportError("Missing ')' in factor");
        getNextToken();
        return val;
    }
    else if (currentToken.type == T_ID)
    {
        char varName = currentToken.ch;
        getNextToken();
        int idx = varName - 'a';
        return variables[idx];
    }
    else if (currentToken.type == T_NUM)
    {
        int val = currentToken.ch - '0';
        getNextToken();
        return val;
    }
    else
        reportError("Unexpected token in parseFactor");
    return 0;
}

static int miniParseE(MiniLexer* ml)
{
    int result = miniParseT(ml);
    while (ml->pos < ml->size)
    {
        Token op = ml->tokens[ml->pos];
        if (op.type == T_PLUS || op.type == T_MINUS)
        {
            ml->pos++;
            int right = miniParseT(ml);
            if (op.type == T_PLUS)
                result += right;
            else
                result -= right;
        }
        else
            break;
    }
    return result;
}
static int miniParseT(MiniLexer* ml)
{
    int result = miniParseU(ml);
    while (ml->pos < ml->size)
    {
        Token op = ml->tokens[ml->pos];
        if (op.type == T_STAR || op.type == T_SLASH || op.type == T_MOD)
        {
            ml->pos++;
            int right = miniParseU(ml);
            if (op.type == T_STAR)
                result *= right;
            else if (op.type == T_SLASH)
            {
                if (right == 0)
                    reportError("miniParseT: divide by zero");
                result /= right;
            }
            else if (op.type == T_MOD)
            {
                if (right == 0)
                    reportError("miniParseT: modulo by zero");
                result %= right;
            }
        }
        else
            break;
    }
    return result;
}
static int miniParseU(MiniLexer* ml)
{
    int left = miniParseF(ml);
    if (ml->pos < ml->size && ml->tokens[ml->pos].type == T_CARET)
    {
        ml->pos++;
        int right = miniParseU(ml);
        int result = 1;
        for (int i = 0; i < right; i++)
            result *= left;
        return result;
    }
    return left;
}
static int miniParseF(MiniLexer* ml)
{
    if (ml->pos >= ml->size)
        reportError("miniParseF: out of tokens");

    Token tk = ml->tokens[ml->pos++];
    if (tk.type == T_LPAREN)
    {
        int val = miniParseE(ml);
        if (ml->pos >= ml->size)
            reportError("miniParseF: missing ')' token");
        Token tk2 = ml->tokens[ml->pos++];
        if (tk2.type != T_RPAREN)
            reportError("miniParseF: missing closing parenthesis");
        return val;
    }
    else if (tk.type == T_ID)
    {
        int idx = tk.ch - 'a';
        return variables[idx];
    }
    else if (tk.type == T_NUM)
    {
        return tk.ch - '0';
    }
    else
        reportError("miniParseF: unexpected token");
    return 0;
}

static void blockParse(BlockParser* bp)
{
    while (bp->pos < bp->size)
    {
        blockStatement(bp);
    }
}

static void blockStatement(BlockParser* bp)
{
    Token tk = blockPeekToken(bp);
    switch (tk.type)
    {
        case T_LBRACKET:
            blockNextToken(bp);
            blockIf(bp);
            break;

        case T_LBRACE:
            blockNextToken(bp);
            blockError("Nested while is not implemented in this example.");
            break;

        case T_ID:
        {
            Token varTok = blockNextToken(bp);
            Token eqTok  = blockNextToken(bp);
            if (eqTok.type != T_ASSIGN)
                blockError("Expected '=' in assignment");
            int val = blockExpr(bp);

            Token semi = blockNextToken(bp);
            if (semi.type != T_SEMI)
                blockError("Expected ';' after assignment");

            if (bp->execFlag)
            {
                int idx = varTok.ch - 'a';
                variables[idx] = val;
            }
        }
        break;

        case T_LT:
        {
            blockNextToken(bp);
            int val = blockExpr(bp);

            Token semi = blockNextToken(bp);
            if (semi.type != T_SEMI)
                blockError("Expected ';' after output");

            if (bp->execFlag)
            {
                printf("%d\n", val);
                fflush(stdout);
            }
        }
        break;

        case T_GT:
        {
            blockNextToken(bp);
            Token varTok = blockNextToken(bp);
            if (varTok.type != T_ID)
                blockError("Expected ID for input statement");

            Token semi = blockNextToken(bp);
            if (semi.type != T_SEMI)
                blockError("Expected ';' after input");

            if (bp->execFlag)
            {
                int val;
                printf("Input for variable '%c': ", varTok.ch);
                fflush(stdout);
                scanf("%d", &val);
                int idx = varTok.ch - 'a';
                variables[idx] = val;
            }
        }
        break;

        case T_END:
            return;

        default:
            blockError("Unexpected token in blockStatement");
    }
}

static void blockIf(BlockParser* bp)
{
    int saved = bp->execFlag;
    int condVal = blockExpr(bp);

    Token qTok = blockNextToken(bp);
    if (qTok.type != T_QUESTION)
        blockError("Missing '?' in blockIf");

    if (condVal != 0)
        bp->execFlag = 1;
    else
        bp->execFlag = 0;

    while (1)
    {
        Token pk = blockPeekToken(bp);
        if (pk.type == T_COLON || pk.type == T_RBRACKET || pk.type == T_END)
            break;
        blockStatement(bp);
    }

    Token pk = blockPeekToken(bp);
    if (pk.type == T_COLON)
    {
        blockNextToken(bp);
        if (condVal != 0)
            bp->execFlag = 0;
        else
            bp->execFlag = 1;

        while (1)
        {
            Token pk2 = blockPeekToken(bp);
            if (pk2.type == T_RBRACKET || pk2.type == T_END)
                break;
            blockStatement(bp);
        }
    }

    Token rb = blockNextToken(bp);
    if (rb.type != T_RBRACKET)
        blockError("Missing ']' in blockIf");

    bp->execFlag = saved;
}

static int blockExpr(BlockParser* bp)
{
    int result = blockTerm(bp);
    while (1)
    {
        Token pk = blockPeekToken(bp);
        if (pk.type == T_PLUS || pk.type == T_MINUS)
        {
            Token op = blockNextToken(bp);
            int right = blockTerm(bp);
            if (op.type == T_PLUS)
                result += right;
            else
                result -= right;
        }
        else
            break;
    }
    return result;
}

static int blockTerm(BlockParser* bp)
{
    int result = blockPower(bp);
    while (1)
    {
        Token pk = blockPeekToken(bp);
        if (pk.type == T_STAR || pk.type == T_SLASH || pk.type == T_MOD)
        {
            Token op = blockNextToken(bp);
            int right = blockPower(bp);
            if (op.type == T_STAR)
                result *= right;
            else if (op.type == T_SLASH)
            {
                if (right == 0)
                    blockError("Division by zero in while block");
                result /= right;
            }
            else
            {
                if (right == 0)
                    blockError("Modulo by zero in while block");
                result %= right;
            }
        }
        else
            break;
    }
    return result;
}

static int blockPower(BlockParser* bp)
{
    int left = blockFactor(bp);

    Token pk = blockPeekToken(bp);
    if (pk.type == T_CARET)
    {
        blockNextToken(bp);
        int right = blockPower(bp);
        int out = 1;
        for(int i = 0; i < right; i++)
            out *= left;
        return out;
    }
    return left;
}

static int blockFactor(BlockParser* bp)
{
    Token tk = blockNextToken(bp);
    if (tk.type == T_LPAREN)
    {
        int val = blockExpr(bp);
        Token tk2 = blockNextToken(bp);
        if (tk2.type != T_RPAREN)
            blockError("Missing ')' in blockFactor");
        return val;
    }
    else if (tk.type == T_ID)
    {
        int idx = tk.ch - 'a';
        return variables[idx];
    }
    else if (tk.type == T_NUM)
    {
        return tk.ch - '0';
    }
    else
        blockError("Unexpected token in blockFactor");
    return 0;
}
static Token blockNextToken(BlockParser* bp)
{
    if (bp->pos >= bp->size)
    {
        Token t;
        t.type = T_END;
        t.ch   = 0;
        return t;
    }
    return bp->tokens[bp->pos++];
}

static Token blockPeekToken(BlockParser* bp)
{
    if (bp->pos >= bp->size)
    {
        Token t;
        t.type = T_END;
        t.ch   = 0;
        return t;
    }
    return bp->tokens[bp->pos];
}

static void blockError(const char* msg)
{
    fprintf(stderr, "While block parse error: %s\n", msg);
    exit(1);
}

static void initTokenBuffer(TokenBuffer* buf)
{
    buf->count    = 0;
    buf->capacity = 16;
    buf->tokens   = (Token*)malloc(sizeof(Token) * buf->capacity);
}

static void freeTokenBuffer(TokenBuffer* buf)
{
    if (buf->tokens)
        free(buf->tokens);
    buf->tokens   = NULL;
    buf->count    = 0;
    buf->capacity = 0;
}

static void pushToken(TokenBuffer* buf, Token tk)
{
    if (buf->count >= buf->capacity)
    {
        buf->capacity *= 2;
        buf->tokens = (Token*)realloc(buf->tokens, sizeof(Token) * buf->capacity);
    }
    buf->tokens[buf->count++] = tk;
}
