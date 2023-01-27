#include "./cc.h"

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *msg)
{
    char *line = loc;
    while (source < line && line[-1] != '\n')
        line--;

    char *end = loc;
    while (*end != '\n')
        end++;

    int line_num = 1;
    for (char *p = source; p < line; p++)
        if (*p == '\n')
            line_num++;

    int indent = fprintf(stderr, "%s:%d: ", file_path, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ %s\n", msg);
    exit(1);
}

bool consume(char *op)
{
    if (
        token->kind == TK_IDENT ||
        token->kind == TK_NUM ||
        token->kind == TK_EOF ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident()
{
    if (token->kind != TK_IDENT)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

void expect(char *op)
{
    if (token->kind == TK_INT && memcmp(op, D_INT, sizeof(D_INT)) == 0)
    {
        token = token->next;
        return;
    }

    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "It is not '%c'.");
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "It is not a number.");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

bool is_alphabet(char *p)
{
    return ('a' <= *p && *p <= 'z');
}

int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

bool is_reserved_keyword(const char *op, const char *keyword)
{
    size_t len = strlen(keyword);
    return strncmp(op, keyword, len) == 0 && !is_alnum(op[len]);
}

Token *tokenize(char *source)
{
    char *p = source;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        // 空白文字をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (startswith(p, "==") ||
            startswith(p, "!=") ||
            startswith(p, "<=") ||
            startswith(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>=;{},&[]", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if ('"' == *p)
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);

            int i = 0;
            while ('"' != *(p + i))
            {
                i += 1;
                continue;
            }
            cur = new_token(TK_STR, cur, p, i);
            p += i;

            assert(*p == '"');
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (is_reserved_keyword(p, D_RETURN))
        {
            size_t keylen = strlen(D_RETURN);
            cur = new_token(TK_RETURN, cur, p, keylen);
            p += keylen;
            continue;
        }

        if (is_reserved_keyword(p, D_IF))
        {
            size_t keylen = strlen(D_IF);
            cur = new_token(TK_IF, cur, p, keylen);
            p += keylen;
            continue;
        }

        if (is_reserved_keyword(p, D_ELSE))
        {
            size_t keylen = strlen(D_ELSE);
            cur = new_token(TK_ELSE, cur, p, keylen);
            p += keylen;
            continue;
        }

        if (is_reserved_keyword(p, D_WHILE))
        {
            size_t keylen = strlen(D_WHILE);
            cur = new_token(TK_WHILE, cur, p, keylen);
            p += keylen;
            continue;
        }

        if (is_reserved_keyword(p, D_FOR))
        {
            size_t keylen = strlen(D_FOR);
            cur = new_token(TK_FOR, cur, p, keylen);
            p += keylen;
            continue;
        }

        if (is_reserved_keyword(p, D_INT))
        {
            size_t keylen = strlen(D_INT);
            cur = new_token(TK_INT, cur, p, keylen);
            p += keylen;
            continue;
        }

        if (is_reserved_keyword(p, D_CHAR))
        {
            size_t keylen = strlen(D_CHAR);
            cur = new_token(TK_CHAR, cur, p, keylen);
            p += keylen;
            continue;
        }

        if (is_reserved_keyword(p, D_SIZEOF))
        {
            size_t keylen = strlen(D_SIZEOF);
            cur = new_token(TK_SIZEOF, cur, p, keylen);
            p += keylen;
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        if (is_alnum(*p))
        {
            int i = 1;
            while (is_alphabet(p + i))
            {
                i += 1;
                continue;
            }

            cur = new_token(TK_IDENT, cur, p, i);
            p += i;
            continue;
        }

        error_at(token->str, "Cannot be tokenized.");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
