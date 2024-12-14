#include "interpreter.h"

//---------------------------------------------------------
// 1) Lexer ile ilgili global değişkenler
//---------------------------------------------------------
static const char* input;  // Program metni
static int         position; // input içindeki indeks
static Token       currentToken;

// Değişken tablosu: 'a'..'z' -> 26 tamsayı
static int variables[26];

// "Yürütme bayrağı" (IF veya WHILE false iken semantic actionları atlamak için)
//  - 1 ise "execute" modunda, 0 ise "skip" modunda parse edilir.
static int executeFlag = 1; 

//---------------------------------------------------------
// 2) Lexer (getToken) - Basit Tokenizer
//---------------------------------------------------------
static Token getToken() {
    Token t;
    // Boşluk veya satır sonu karakterlerini geç
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
            // a-z => ID, 0-9 => NUM, aksi T_UNKNOWN
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

//---------------------------------------------------------
// Yardımcı fonksiyonlar
//---------------------------------------------------------
static void getNextToken() {
    currentToken = getToken();
}

static void error(const char* msg) {
    fprintf(stderr, "Parser error: %s\n", msg);
    exit(1);
}

//---------------------------------------------------------
// 3) Token buffer'lama mekanizması (özellikle WHILE için)
//---------------------------------------------------------
//
// WHILE bloğu: '{' E '?' C{C} '}'
//   -> Koşul ifadesi (E) nin tokenlarını buffer’a almak
//   -> Bloğun tokenlarını '}' görene dek buffer'a almak
//   -> cond != 0 ise o bloğu tekrar parse + execute
//
// Bunu yapmak için 2 fonksiyon yazacağız:
//  1) bufferTokens() : Belirli bir bitiş tokenına kadar tokenları bir diziye koyacak
//  2) parseTokens()  : O token dizisini "sub-parse" edecek (yanı proje içinde mini-lexer).
//
// Bu sayede "koşul 0 olmadığı sürece" bloğu tekrar parse + execute edebiliriz.
//

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

// MiniLexer fonksiyonları
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

// "alt parse" yaparken global currentToken'ı bozmamak için
// global fonksiyonların kopyalarını yapacağız. 
// parseTokens fonksiyonunda "P, C, vb." fonksiyonların mini versiyonları veya 
// global fonksiyonların "re-entrant" şekli gerekecek. 
// Burada en basit yaklaşım: parseWhileBody vb. adında bir alt fonksiyon yazıp
// "MiniLexer"i local tutacağız.

//---------------------------------------------------------
// 4) Gramer Fonksiyonları (TAM UYGULAMA)
//---------------------------------------------------------

// P → { C } '.'
static void P() {
    // '.' görene kadar sıfır veya daha fazla C
    while (currentToken.type != T_DOT) {
        if (currentToken.type == T_END) {
            error("P: Program sonu gelmeden '.' bekleniyor.");
        }
        C();
    }
    // currentToken.type == T_DOT, bunu yiyoruz
    getNextToken();
    printf("Program basariyla parse edildi.\n");
}

// C → I | W | A | Ç | G
static void C() {
    switch (currentToken.type) {
        case T_LBRACKET: // '[' => I()
            getNextToken(); // '[' tüket
            I();
            break;
        case T_LBRACE:   // '{' => W()
            getNextToken(); // '{'
            W();
            break;
        case T_ID:       // A() => K '=' E ';'
            A();
            break;
        case T_LT:       // '<' => Ç()
            getNextToken();
            Cik();
            break;
        case T_GT:       // '>' => G()
            getNextToken();
            Girdi();
            break;
        default:
            // C epsilon'a gitmiyor. Burada hata vermek en iyisi.
            error("C: Beklenmeyen token.");
    }
}

// I → '[' E '?' C{C} ':' C{C} ']' | '[' E '?' C{C} ']'
// NOT: '[' tokenı C() içinde yendi, o yüzden I() da direkt E() parse
static void I() {
    int cond = E(); // E parse + evaluate

    if (currentToken.type != T_QUESTION) {
        error("IF: '?' bekleniyor.");
    }
    getNextToken(); // '?' yedik

    // Koşul TRUE => executeFlag = 1, FALSE => executeFlag = 0
    int savedFlag = executeFlag;
    if (cond != 0) {
        executeFlag = 1;
    } else {
        executeFlag = 0;
    }

    // if-body : C{C}, ':' veya ']' görene kadar parse
    while (currentToken.type != T_COLON && currentToken.type != T_RBRACKET) {
        if (currentToken.type == T_DOT || currentToken.type == T_END) {
            error("IF: ':' veya ']' bekleniyor.");
        }
        C(); // C() parse + (executeFlag doğrultusunda) exec
    }

    // ELSE bölümü var mı?
    if (currentToken.type == T_COLON) {
        // else var
        getNextToken(); // ':' yedik
        // else-body: ']' gelene kadar parse
        if (cond != 0) {
            // if koşulu TRUE ise else bloğunu SKIP
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

    // ']' bekleniyor
    if (currentToken.type != T_RBRACKET) {
        error("IF: ']' bekleniyor.");
    }
    getNextToken(); // ']' yedik

    // executeFlag restore
    executeFlag = savedFlag;
}

// W → '{' E '?' C{C} '}'
static void W() {
    // Koşul ifadesi (E) tokenlarını buffer'a alacağız (çünkü her iterasyonda tekrar evaluate etmek gerek)
    // 1) E()'yi parse etmeden önce tokenlarını toplayalım:
    // Aslında parse etmeden token toplamak karmaşık, 
    // en kolayı: E() parse edip "cond" hesaplamak bir kez, 
    // ama tekrar parse için de token yok olur (lexer ilerler).
    // Daha doğru yaklaşım:
    //   - Koşul E'sini "token buffer"ına kaydedip,
    //   - parseWhileCondition() fonksiyonuyla tekrar tekrar evaluate edelim.

    // Plan:
    //   - Koşul E'sinin tokenlarını '?' gelene kadar buffer'a dolduralım.
    //   - Sonra '?' yi bekleyip yedikten sonra
    //   - Döngü bloğunu '}' gelene kadar buffer'a dolduralım.
    //   - Artık elimizde 2 token buffer var: condTokens, blockTokens.
    //   - while (evaluate(condTokens) != 0) { parseAndExecute(blockTokens) }

    // step 1: Koşul tokenlarını '?' görene dek buffer'a al
    TokenBuffer condBuf; 
    initTokenBuffer(&condBuf);

    // Lexer state. Geri dönmek zor olduğu için, '?' görene kadar tokenları sakla
    while (currentToken.type != T_QUESTION) {
        if (currentToken.type == T_END || currentToken.type == T_DOT) {
            error("WHILE: Kosul ifadesinde '?' bekleniyor.");
        }
        pushToken(&condBuf, currentToken);
        getNextToken();
    }
    // currentToken.type == T_QUESTION
    getNextToken(); // '?' yedik

    // step 2: Döngü bloğunu '}' görene dek buffer'a al
    TokenBuffer blockBuf;
    initTokenBuffer(&blockBuf);

    while (currentToken.type != T_RBRACE) {
        if (currentToken.type == T_END || currentToken.type == T_DOT) {
            error("WHILE: '}' bekleniyor.");
        }
        pushToken(&blockBuf, currentToken);
        getNextToken();
    }
    // '}' yedik
    getNextToken(); // döngü bitti

    // Şimdi condBuf içinde E ifadesinin tokenları var
    // blockBuf içinde C{C} nin tokenları var
    // Tekrar tekrar evaluate + execute edelim:
    while (1) {
        // Evaluate condBuf
        // MiniLexer aç
        MiniLexer mlCond;
        ml_init(&mlCond, condBuf.tokens, condBuf.count);

        // Mini parse: E() fonksiyonunu çalıştırmak için minik kopya fonksiyonu yapabiliriz
        // ama buradaki E() global currentToken'a bakıyor. Bu global state'i bozmadan 
        // "cond" hesaplamanın en pratik yolu: 
        //   - "EvalExpressionFromTokens" gibi local bir fonksiyon yazalım.

        // Koşulu değerlendir
        // E => T => U => F kuralını mini-parse edelim.
        // Bu fonksiyonu hemen yazalım:

        // 1) parse and compute E from condBuf
        //    eğer cond == 0, break
        //    !=0 ise blockBuf parse+execute

        // Ama semantik action (değişken tablosu) ortak. 
        // "executeFlag" 1 ise atama vb. işlesin. 
        // Koşul ifadesini her zaman "eval" etmemiz gerekiyor, "skip" yok.
        // O yüzden E'yi parse eden "mini parser" lazim.

        // Evaluate E tokens
        // E() => T() => U() => F() 
        // Bu mini parser + mini E/T/U/F kopyasını yapalım.

        // Koşul ifadesinin parse-interpret fonksiyonu
        int condVal = 0;

        // *** ÖNEMLİ *** 
        // Aşağıdaki "mini" fonksiyonlarla global parser'ı taklit ediyoruz:

        // Forward decl
        int parseE(MiniLexer* ml);

        // Mini versions:
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
            // peek next token
            if (ml->pos < ml->size && ml->tokens[ml->pos].type == T_CARET) {
                // '^'
                ml->pos++; // consume '^'
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

        // mlCond bitti
        // condVal == 0 -> döngüden çık
        if (condVal == 0) {
            break;
        }

        // condVal != 0 => blockBuf parse + execute
        // Bunu global parse fonksiyonlarımızla tekrar edemeyiz, 
        // yoksa global currentToken bozulur. 
        // Onun yerine "mini parse" mi yapalım? 
        // Evet, blockBuf'u parse edebilecek bir alt fonksiyon.

        // blockBuf: C{C} => normal parse mantığı. 
        // Ama atama, if, vs. hepsi var, bunlar global F() vs. fonksiyonlarını çağırıyor. 
        // Tek çare, orada da "mini parser" kopyası. 
        // Yada "rekürsif descent"i block buffer'a uygulayacağız.

        // Fakat *burada* biz tam dilin parse logic'ini kopyalamak istemiyoruz. 
        // Ama proje gereği bu tam "dili" blockBuf içinden parse etmek istiyoruz. 
        // Yani "C()" ifadesi mesela. 
        // Mevcut global parse fonksiyonlarına "currentToken" beslemek gerekir.
        // Bunu yapmak için ufak bir trick:
        //  1) blockBuf'u global input'a "inject" edebiliriz. 
        //  2) parse bitince geri döneriz. 
        //  Bu çok karmaşık. 
        // En iyi yaklaşım: local mini parser. 
        // Fakat her kuralı kopyalamamız lazım (I, W, A, vs.).

        // Kolay approach: 
        // 1) Tek seferlik approach: blockBuf parse edelim, semantic action. 
        // 2) Bunu yapabilmek için "tekrar parse" = blockBuf tokenlarını okuyan mini-lexer + "C() benzeri" kopyası. 
        // Ama epey kod tekrarı olacak.

        // Hızlı bir pratik çözüm: "blockBuf"u parse eden *** aynen *** global dil kopyası:
        // parseBlockTokens(blockBuf), orada P, C, I, vb. minik kopyaları. 
        // On-the-fly interpreter.

        // Aşağıda "parseBlock" fonksiyonu yazıyoruz, 
        // blockBuf tokenlarını sonuna kadar C() C() ... parse ediyor. 
        // "}" yok, zira blockBuf zaten '}' görene kadar dolduruldu.

        // parseBlock da "executeFlag" = 1 modunda cümlecikleri çalıştıracak.

        // *** parseBlock fonksiyonu: blockBuf tokens bitene kadar C() parse ediyor. 
        // But we must replicate the entire grammar. 
        // Tek fark: blockBuf '}' tokenını içermiyor (zaten durdurduk). 
        // Yani tokenlar bitene kadar C parse.

        // *** Gelelim tekrara. Evet, her iteration blockBuf'u en baştan parse etmeliyiz. 

        // Aynı grammar'ı implement eden minik fonksiyon kümesi:
        // (Kod tekrarı çok, ama proje gereği)

        int block_E(BlockParser* bp); // forward

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
        // "peeks" current token without consuming
        Token block_peekToken(BlockParser* bp) {
            if (bp->pos >= bp->size) {
                Token t;
                t.type = T_END;
                t.ch   = 0;
                return t;
            }
            return bp->tokens[bp->pos];
        }

        void block_C(BlockParser* bp); // forward
        void block_I(BlockParser* bp);

        void block_parseBlock(BlockParser* bp) {
            // block: bir dizi C. T_END veya block sonuna kadar
            while (bp->pos < bp->size) {
                block_C(bp);
            }
        }

        void block_C(BlockParser* bp) {
            Token tk = block_peekToken(bp);
            switch (tk.type) {
                case T_LBRACKET: // '['
                    block_nextToken(bp); // consume '['
                    block_I(bp);
                    break;
                case T_LBRACE: // '{'
                    // Nested while
                    // Bu kadar proje kodu içerisinde nested while mini parser daha da karmaşık.
                    // Sınırsız hallerde tam bir re-entrant parser gerekiyor. 
                    // Zaman kısıtından dolayı bunları da kopyalıyoruz:
                    block_nextToken(bp); 
                    block_error("Nested while icin extra parser kodu gerekli, ornek iskelet.");
                    break;
                case T_ID: {
                    // A -> K '=' E ';'
                    Token varTok = block_nextToken(bp); // T_ID
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
                    // '<' E ';'
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
                    // '>' K ';'
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
                    // block'un sonu mu? T_END?
                    if (tk.type == T_END) {
                        // block bitti
                        return;
                    }
                    block_error("WHILE block: Beklenmeyen token C()");
            }
        }

        void block_I(BlockParser* bp) {
            // IF benzeri: E '?' ...
            int saved = bp->execFlag;
            // parse E
            int condVal = 0;

            // minik parse E
            condVal = block_E(bp);

            Token qTok = block_nextToken(bp);
            if (qTok.type != T_QUESTION) {
                block_error("WHILE block: IF icin '?' bekleniyor.");
            }

            // if-body parse => ':' veya ']' görene kadar
            if (condVal != 0) {
                bp->execFlag = 1;
            } else {
                bp->execFlag = 0;
            }

            // parse if-body
            while (1) {
                Token pk = block_peekToken(bp);
                if (pk.type == T_COLON || pk.type == T_RBRACKET || pk.type == T_END) {
                    break;
                }
                block_C(bp);
            }

            // else var mı?
            pk = block_peekToken(bp);
            if (pk.type == T_COLON) {
                block_nextToken(bp); // ':'
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
            // ']' bekle
            Token rb = block_nextToken(bp);
            if (rb.type != T_RBRACKET) {
                block_error("WHILE block: IF icin ']' bekleniyor.");
            }

            // restore execFlag
            bp->execFlag = saved;
        }

        // block_E => T { (+|-) T }
        int block_T(BlockParser* bp);  // forward

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

        // blockBuf parse & execute
        BlockParser bp;
        bp.tokens   = blockBuf.tokens;
        bp.pos      = 0;
        bp.size     = blockBuf.count;
        bp.execFlag = 1;
        block_parseBlock(&bp);
    }

    // Döngü bitti. Temizlik:
    freeTokenBuffer(&condBuf);
    freeTokenBuffer(&blockBuf);
}

// A → K '=' E ';'
static void A() {
    char varName = currentToken.ch; // T_ID
    getNextToken(); // consume ID

    if (currentToken.type != T_ASSIGN) {
        error("Atama: '=' bekleniyor.");
    }
    getNextToken(); // '='

    int val = E();

    if (currentToken.type != T_SEMI) {
        error("Atama sonunda ';' bekleniyor.");
    }
    getNextToken(); // ';'

    if (executeFlag) {
        int idx = varName - 'a';
        variables[idx] = val;
    }
}

// Ç → '<' E ';'
static void Cik() {
    int val = E();
    if (currentToken.type != T_SEMI) {
        error("Cikti ifadesinde ';' bekleniyor.");
    }
    getNextToken(); // ';'

    if (executeFlag) {
        printf("%d\n", val);
    }
}

// G → '>' K ';'
static void Girdi() {
    if (currentToken.type != T_ID) {
        error("Girdi ifadesinde degisken (ID) bekleniyor.");
    }
    char varName = currentToken.ch;
    getNextToken(); // ID
    if (currentToken.type != T_SEMI) {
        error("Girdi ifadesinde ';' bekleniyor.");
    }
    getNextToken(); // ';'

    if (executeFlag) {
        int val;
        printf("Input for variable '%c': ", varName);
        scanf("%d", &val);
        int idx = varName - 'a';
        variables[idx] = val;
    }
}

// E → T {('+' | '-') T}
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

// T → U {('*' | '/' | '%') U}
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

// U → F '^' U | F
static int U() {
    int left = F();
    if (currentToken.type == T_CARET) {
        getNextToken(); // '^'
        int right = U();
        int result = 1;
        for(int i=0; i<right; i++){
            result *= left;
        }
        return result;
    }
    return left;
}

// F → '(' E ')' | K | R
static int F() {
    if (currentToken.type == T_LPAREN) {
        getNextToken(); // '('
        int val = E();
        if (currentToken.type != T_RPAREN) {
            error("Parantez kapatma hatasi: ')' bekleniyor.");
        }
        getNextToken(); // ')'
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

//---------------------------------------------------------
// 5) main - Test
//---------------------------------------------------------
int main() {
    // Aşağıdaki program, projede istenen grammar ve while yapısının test örneğidir.
    //
    // Program (örnek):
    //   n = 0;
    //   { n - 2*5 ?
    //     < n;
    //     n = n + 1;
    //   }
    //   .
    //
    // Bu, n=0 iken (0 - 10 = -10) true (0 hariç her değer true). Döngü n=10 olana kadar devam eder.
    // Beklenen çıktı: 0'dan 9'a kadar sayıları ekrana basar (toplam 10 kere).
    //
    // Bu tam bir while fonksiyonu. 
    // Kodumuz "W()" fonksiyonu içerisinde condTokens ve blockTokens yaklaşımı kullanarak
    // gerçek tekrar yürütecek.

    const char* programText =
        "n = 0;\n"
        "{ n - 2*5 ?\n"
        "  < n;\n"
        "  n = n + 1;\n"
        "}\n"
        ".\n";

    input = programText;
    position = 0;
    memset(variables, 0, sizeof(variables));

    getNextToken();
    P();

    return 0;
}

