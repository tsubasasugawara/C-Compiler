#include "./cc.h"

void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("The left-hand side value of the assignment is not a variable.");

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

int label = 0;

void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        int if_label = label++;

        gen(node->condition);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");

        if (node->els)
        {
            printf("    je .Lelse%03d\n", if_label);
            gen(node->then);
            printf("    jmp .Lend%03d\n", if_label);
            printf(".Lelse%03d:\n", if_label);
            gen(node->els);
            printf(".Lend%03d:\n", if_label);
        }
        else
        {
            printf("    je .Lend%03d\n", if_label);
            gen(node->then);
            printf(".Lend%03d:\n", if_label);
        }
        return;
    case ND_WHILE:
        int while_label = label++;

        printf(".Lbegin%03d:\n", while_label);
        gen(node->condition);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%03d\n", while_label);
        gen(node->body);
        printf("    jmp .Lbegin%03d\n", while_label);
        printf(".Lend%03d:\n", while_label);

        return;
    case ND_FOR:
        int for_label = label++;

        if (node->init)
        {
            gen(node->init);
        }
        printf(".Lbegin%03d:\n", for_label);
        if (node->condition)
        {
            gen(node->condition);
        }
        else
        {
            printf("    push 0\n");
        }
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%03d\n", for_label);
        gen(node->body);
        if (node->update)
        {
            gen(node->update);
        }
        printf("   jmp .Lbegin%03d\n", for_label);
        printf(".Lend%03d:\n", for_label);

        return;
    case ND_BLOCK:
        for (int i = 0; i < node->stmts->len; i++)
            gen(node->stmts->data[i]);
        return;
    case ND_CALL:
        for (int i = 0; i < node->args->len; i++)
        {
            gen(node->args->data[i]);
            char *register_name;
            switch (i)
            {
            case 0:
            {
                register_name = "rdi";
                break;
            }
            case 1:
            {
                register_name = "rsi";
                break;
            }
            case 2:
            {
                register_name = "rdx";
                break;
            }
            case 3:
            {
                register_name = "rcx";
                break;
            }
            case 4:
            {
                register_name = "r8";
                break;
            }
            case 5:
            {
                register_name = "r9";
                break;
            }
            }
            printf("    pop rax\n");
            printf("    mov %s, rax\n", register_name);
        }
        printf("    call %s\n", node->name);
        printf("    push rax\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
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
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
    return;
}

void gen_func(Function *func)
{
    // 関数定義のプロローグ
    printf(".globl %s\n", func->name);
    printf("%s:\n", func->name);

    // 変数のスペースを確保
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", func->lvars->len * 8);

    gen(func->node->body);

    // Epilogue
    // 最後の式の結果はraxにあり、返り値となる
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return;
}

void codegen()
{
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    // 先頭の式から順にコード生成
    for (int i = 0; i < program->funcs->len; i++)
    {
        gen_func(program->funcs->data[i]);
    }

    return;
}
