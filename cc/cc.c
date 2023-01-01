#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    TK_RESERVED, // �L��
    TK_NUM,      // �����g�[�N��
    TK_EOF,      // ���͂̏I����\���g�[�N��
} TokenKind;

typedef struct Token Token;

// �g�[�N���^
struct Token
{
    TokenKind kind; // �g�[�N���̌^
    Token *next;    // ���̓��̓g�[�N��
    int val;        // kind��TK_NUM�̏ꍇ�A���̐��l
    char *str;      // �g�[�N��������
};

// ���ݒ��ڂ��Ă���g�[�N��
Token *token;

// ���̓v���O����
char *user_input;

// �G���[�ӏ���񍐂���
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos�̋󔒂��o��
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// ���̃g�[�N�������҂��Ă���L���̂Ƃ��ɂ́A�g�[�N����1�ǂݐi�߂�
// �^��Ԃ��B����ȊO�̏ꍇ�ɂ͋U��Ԃ��B
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

// ���̃g�[�N�������҂��Ă���L���̂Ƃ��ɂ́A�g�[�N����1�ǂݐi�߂�B
// ����ȊO�̏ꍇ�ɂ̓G���[��񍐂���B
void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error_at(token->str, "It is not '%c'.");
    token = token->next;
}

// ���̃g�[�N�������l�̏ꍇ�A�g�[�N����1�ǂݐi�߂Ă��̐��l��Ԃ��B
// ����ȊO�̏ꍇ�ɂ̓G���[��񍐂���B
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

// �V�����g�[�N�����쐬����cur�Ɍq����
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// ���͕�����p���g�[�N�i�C�Y���Ă����Ԃ�
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        // �󔒕������X�L�b�v
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-')
        {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(token->str, "Cannot be tokenized.");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

// ���ۍ\���؂̃m�[�h�̎��
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // ����
} NodeKind;

typedef struct Node Node;

// ���ۍ\���؂̃m�[�h�̌^
struct Node {
    NodeKind kind;  // �m�[�h�̌^
    Node *lhs;      // ����
    Node *rhs;      // �E��
    int val;        // kind��ND_NUM�̏ꍇ�̂ݎg��
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *mul();
Node *primary();

Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = primary();

    for (;;) {
        if (consume('*'))
            node = new_node(ND_MUL, node, primary());
        else if (consume('/'))
            node = new_node(ND_DIV, node, primary());
        else
            return node;
    }
}

Node *primary() {
    //���̃g�[�N����"("�Ȃ�A"(" epr ")"�̂͂�
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }

    // �����łȂ���ΐ��l�̂͂�
    return new_node_num(expect_number());
}

void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "The number of arguments is incorrect.\n");
        return 1;
    }

    user_input = argv[1];

    // �g�[�N�i�C�Y
    token = tokenize(argv[1]);

    // �A�Z���u���̑O���������o��
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // ���̍ŏ��͐��łȂ���΂Ȃ�Ȃ��̂ŁA������`�F�b�N����
    // �ŏ���mov���߂��o��
    printf("    mov rax, %d\n", expect_number());

    // `+ <��>`���邢��`- <��>`�Ƃ����g�[�N���̕��т������
    // �A�Z���u�����o��
    while (!at_eof())
    {
        if (consume('+'))
        {
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}
