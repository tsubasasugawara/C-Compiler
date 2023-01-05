#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------�\��� ------------------------------ */
#define D_RETURN "return"
#define D_IF "if"
#define D_ELSE "else"
#define D_WHILE "while"
#define D_FOR "for"

/* ------------------------------�g�[�N�i�C�U�[ ------------------------------ */

typedef enum
{
    TK_RESERVED, // �L��
    TK_IDENT,    // ���ʎq
    TK_NUM,      // �����g�[�N��
    TK_EOF,      // ���͂̏I����\���g�[�N��
    TK_RETURN,   // return
    TK_IF,       // if
    TK_ELSE,     // else
    TK_WHILE,    // while
    TK_FOR,      // for
} TokenKind;

typedef struct Token Token;

// �g�[�N���^
struct Token
{
    TokenKind kind; // �g�[�N���̌^
    Token *next;    // ���̓��̓g�[�N��
    int val;        // kind��TK_NUM�̏ꍇ�A���̐��l
    char *str;      // �g�[�N��������
    int len;        // ������̒���
};

// ���ݒ��ڂ��Ă���g�[�N��
extern Token *token;

// ���̓v���O����
extern char *user_input;

// �G���[��񍐂��邽�߂̊֐�
// printf�Ɠ������������
void error(char *fmt, ...);

// �G���[�ӏ���񍐂���
void error_at(char *loc, char *fmt, ...);

// ���̃g�[�N�������҂��Ă���L���̂Ƃ��ɂ́A�g�[�N����1�ǂݐi�߂�
// �^��Ԃ��B����ȊO�̏ꍇ�ɂ͋U��Ԃ��B
bool consume(char *op);

Token *consume_ident();

// ���̃g�[�N�������҂��Ă���L���̂Ƃ��ɂ́A�g�[�N����1�ǂݐi�߂�B
// ����ȊO�̏ꍇ�ɂ̓G���[��񍐂���B
void expect(char *op);

// ���̃g�[�N�������l�̏ꍇ�A�g�[�N����1�ǂݐi�߂Ă��̐��l��Ԃ��B
// ����ȊO�̏ꍇ�ɂ̓G���[��񍐂���B
int expect_number();

bool at_eof();

// �V�����g�[�N�����쐬����cur�Ɍq����
Token *new_token(TokenKind kind, Token *cur, char *str, int len);

// ���͕�����p���g�[�N�i�C�Y���Ă����Ԃ�
Token *tokenize();

/* ------------------------------ �p�[�T�[ ------------------------------ */

typedef struct
{
    void **data;
    int capacity;
    int len;
} Vector;

Vector *new_vec();
void vec_push(Vector *v, void *elem);
void *vec_pop(Vector *v);

// ���ۍ\���؂̃m�[�h�̎��
typedef enum
{
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_ASSIGN, // =
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_LVAR,   // ���[�J���ϐ�
    ND_NUM,    // ����
    ND_RETURN, // return
    ND_IF,     // if
    ND_WHILE,  // while
    ND_FOR,    // for
    ND_BLOCK,  // {}�̒�
} NodeKind;

typedef struct Node Node;

// ���ۍ\���؂̃m�[�h�̌^
struct Node
{
    NodeKind kind; // �m�[�h�̌^
    Node *next;    // ���̃m�[�h
    Node *lhs;     // ����
    Node *rhs;     // �E��

    // "if(" condition ")" then "else" els
    // "while (" condition ")" body
    // "for (" init ";" condition ";" update ";)" body
    Node *condition;
    Node *then;
    Node *els;
    Node *body;
    Node *init;
    Node *update;
    Vector *stmts;

    int val;    // kind��ND_NUM�̏ꍇ�̂ݎg��
    int offset; // kind��ND_LVAR�̏ꍇ�̂ݎg��
};

typedef struct LVar LVar;

// ���[�J���ϐ��̌^
struct LVar
{
    LVar *next; // ���̕ϐ���NULL
    char *name; // �ϐ��̖��O
    int len;    // ���O�̒���
    int offset; // RBP����̃I�t�Z�b�g
};

void program();

/* ------------------------------ �R�[�h�W�F�l���[�^ ------------------------------ */

void gen(Node *node);

/* ------------------------------ �O���[�o���ϐ� ------------------------------ */
// ���ݒ��ڂ��Ă���g�[�N��
extern Token *token;
// ���̓v���O����
extern char *user_input;
extern Node *code[100];
