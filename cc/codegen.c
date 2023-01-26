#include "./cc.h"

void gen(Node *node);

void gen_lval(Node *node)
{
    switch (node->kind)
    {
    case ND_LVAR:
        printf("    mov rax, rbp\n");
        printf("    sub rax, %d\n", node->offset);
        printf("    push rax\n");
        return;
    case ND_DEREF:
        gen(node->lhs);
        return;
    case ND_GVAR:
        printf("    lea rax, %s[rip]\n", node->name);
        printf("    push rax\n");
        return;
    }

    error("The left-hand side value of the assignment is not a variable.");
}

int label = 0;

void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_GVAR:
    case ND_LVAR:
        gen_lval(node);
        if (node->type->ty != TY_ARRAY)
        {
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
        }
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
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
            char *register_name = get_register_name(i);
            printf("    pop rax\n");
            printf("    mov %s, rax\n", register_name);
        }
        printf("    call %s\n", node->name);
        printf("    push rax\n");
        return;
    case ND_ADDR:
        gen_lval(node->lhs);
        return;
    case ND_DEREF:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
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
        if ((node->lhs->type->ty == TY_PTR || node->lhs->type->ty == TY_ARRAY) &&
            !(node->rhs->type->ty == TY_PTR || node->rhs->type->ty == TY_ARRAY))
            printf("    imul rdi, %d\n", size_of(node->lhs->type->ptr_to));

        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        if ((node->lhs->type->ty == TY_PTR || node->lhs->type->ty == TY_ARRAY) &&
            !(node->rhs->type->ty == TY_PTR || node->rhs->type->ty == TY_ARRAY))
            printf("    imul rdi, %d\n", size_of(node->lhs->type->ptr_to));

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

void gen_gvar(Var *gvar)
{
    printf("    .globl %s\n", gvar->name);
    printf("%s:\n", gvar->name);
    printf("    .zero %d\n", calc_need_byte(gvar->type));
}

void gen_func(Function *func)
{
    // 関数定義のプロローグ
    printf("    .globl %s\n", func->name);
    printf("%s:\n", func->name);

    // 変数のスペースを確保
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    if (func->lvars->len > 0)
    {
        Var *last = vec_last(func->lvars);
        printf("    sub rsp, %d\n", last->offset);
    }

    for (int i = 0; i < func->node->params->len; i++)
    {
        gen_lval(func->node->params->data[i]);
        char *register_name = get_register_name(i);
        printf("    pop rax\n");
        printf("    mov [rax], %s\n", register_name);
        printf("    push rdi\n");
    }

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
    printf("    .intel_syntax noprefix\n");

    printf("    .bss\n");
    for (int i = 0; i < program->gvars->keys->len; i++)
    {
        gen_gvar(program->gvars->elems->data[i]);
    }

    // 先頭の式から順にコード生成
    printf("    .text\n");
    for (int i = 0; i < program->funcs->len; i++)
    {
        gen_func(program->funcs->data[i]);
    }

    return;
}
