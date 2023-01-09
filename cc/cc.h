#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ------------------------------予約語 ------------------------------ */
#define D_RETURN "return"
#define D_IF "if"
#define D_ELSE "else"
#define D_WHILE "while"
#define D_FOR "for"
#define D_INT "int"
#define D_SIZEOF "sizeof"

/* ------------------------------トークナイザー ------------------------------ */

typedef enum
{
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
    TK_RETURN,   // return
    TK_IF,       // if
    TK_ELSE,     // else
    TK_WHILE,    // while
    TK_FOR,      // for
    TK_INT,      // int型
    TK_SIZEOF,   // sizeof
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token
{
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // 文字列の長さ
};

// 現在着目しているトークン
extern Token *token;

// 入力プログラム
extern char *user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...);

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...);

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op);

Token *consume_ident();

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op);

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number();

bool at_eof();

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len);

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize();

/* ------------------------------ パーサー ------------------------------ */

typedef struct Vector Vector;

struct Vector
{
    void **data;
    int capacity;
    int len;
};

Vector *new_vec();
void vec_push(Vector *v, void *elem);
void *vec_pop(Vector *v);

typedef struct Type Type;

struct Type
{
    enum
    {
        INT,
        PTR
    } ty;
    struct Type *ptr_to;
};

// 抽象構文木のノードの種類
typedef enum
{
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_ADDR,   // 単項&
    ND_DEREF,  // 単項*
    ND_ASSIGN, // =
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_LVAR,   // ローカル変数
    ND_NUM,    // 整数
    ND_RETURN, // return
    ND_IF,     // if
    ND_WHILE,  // while
    ND_FOR,    // for
    ND_BLOCK,  // {}の中
    ND_CALL,   // 関数呼び出し
    ND_FUNC,   // 関数定義
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node
{
    NodeKind kind; // ノードの型
    Node *next;    // 次のノード
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺

    // "if(" condition ")" then "else" els
    // "while (" condition ")" body
    // "for (" init ";" condition ";" update ";)" body
    Node *condition;
    Node *then;
    Node *els;
    Node *body;
    Node *init;
    Node *update;
    Vector *stmts;  // ブロックの中
    Vector *args;   // 引数
    Vector *params; // 関数のパラメタ

    int val;    // kindがND_NUMの場合のみ使う
    int offset; // kindがND_LVARの場合のみ使う
    char *name; // kindがND_CALLの場合のみ使う
    Type *type; // 変数のときに型を格納
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar
{
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
    Type *type; // 型
};

typedef struct Function Function;

struct Function
{
    char *name;
    Node *node;
    Vector *lvars;
};

typedef struct Program Program;

struct Program
{
    Vector *funcs;
};

Program *parse();

/* ------------------------------ コードジェネレータ ------------------------------ */

void codegen();

/* ------------------------------ グローバル変数 ------------------------------ */
// 現在着目しているトークン
extern Token *token;
// 入力プログラム
extern char *user_input;
extern Program *program;
extern char *register_list_for_arguments[6];

/* ------------------------------ ユーティリティ ------------------------------ */
void swap_node(Node **p, Node **q);
int size_of(Type *type);
