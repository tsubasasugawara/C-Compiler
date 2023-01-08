#include "./cc.h"

// ローカル変数
Vector *lvars;

LVar *find_lvar(Token *tok)
{
    for (int i = 0; i < lvars->len; i++)
    {
        LVar *var;
        var = lvars->data[i];
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    }
    return NULL;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

LVar *new_lvar(Token *tok, Type *type)
{
    LVar *lvar;
    lvar = calloc(1, sizeof(LVar));
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->offset = (lvars->len + 1) * 8;
    lvar->type = type;
    return lvar;
}

Program *parse();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Program *parse()
{
    Program *program;
    program = calloc(1, sizeof(Program));
    Vector *funcs = new_vec();

    while (!at_eof())
    {
        expect(D_INT);
        Token *tok = consume_ident();
        if (!tok)
            error_at(tok->str, "A top-level function definition is required.");

        Node *node;
        Vector *params;
        Function *func;

        node = calloc(1, sizeof(Node));
        func = calloc(1, sizeof(Function));
        lvars = new_vec();
        params = new_vec();

        node->kind = ND_FUNC;

        char *name = strndup(tok->str, tok->len);
        node->name = name;
        func->name = name;

        expect("(");
        while (!consume(")"))
        {
            expect(D_INT);
            Token *param_tok = consume_ident();
            if (!param_tok || param_tok->kind != TK_IDENT)
                error_at(param_tok->str, "Arguments required.");

            size_t param_len = sizeof(register_list_for_arguments) / sizeof(register_list_for_arguments[0]);
            if (params->len > param_len)
                error_at(param_tok->str, "Only up to %d function arguments are supported.", param_len);

            LVar *lvar;
            lvar = calloc(1, sizeof(LVar));
            lvar->name = param_tok->str;
            lvar->len = param_tok->len;
            lvar->offset = (lvars->len + 1) * 8;
            vec_push(lvars, lvar);

            Node *param_node;
            param_node = calloc(1, sizeof(Node));
            param_node->kind = ND_LVAR;
            param_node->offset = (params->len + 1) * 8;
            vec_push(params, param_node);
            consume(",");
        }

        node->body = stmt();
        node->params = params;
        func->lvars = lvars;
        func->node = node;
        vec_push(funcs, func);
    }

    program->funcs = funcs;
    return program;
}

Node *stmt()
{
    Node *node;

    if (consume(D_RETURN))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
    }
    else if (consume(D_FOR))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        if (!consume(";"))
        {
            node->init = expr();
            expect(";");
        }
        if (!consume(";"))
        {
            node->condition = expr();
            expect(";");
        }
        if (!consume(")"))
        {
            node->update = expr();
            expect(")");
        }
        node->body = stmt();
    }
    else if (consume(D_WHILE))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->condition = expr();
        expect(")");
        node->body = stmt();
    }
    else if (consume(D_IF))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->condition = expr();
        expect(")");
        node->then = stmt();

        if (consume(D_ELSE))
        {
            node->els = stmt();
            consume(";");
        }
    }
    else if (consume("{"))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        Vector *stmts = new_vec();

        while (!consume("}"))
        {
            vec_push(stmts, stmt());
        }

        node->stmts = stmts;
    }
    else
    {
        node = expr();
        expect(";");
    }

    return node;
}

Node *expr()
{
    return assign();
}

Node *assign()
{
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational()
{
    Node *node = add();

    for (;;)
    {
        if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary()
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    if (consume("&"))
        return new_node(ND_ADDR, unary(), NULL);
    if (consume("*"))
        return new_node(ND_DEREF, unary(), NULL);
    return primary();
}

Node *primary()
{
    // 次のトークンが"("なら、"(" epr ")"のはず
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok)
    {
        if (consume("("))
        {
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_CALL;
            node->name = strndup(tok->str, tok->len);
            node->args = new_vec();

            while (!consume(")"))
            {
                consume(",");
                Node *arg = expr();
                vec_push(node->args, arg);

                size_t arg_len = sizeof(register_list_for_arguments) / sizeof(register_list_for_arguments[0]);
                if (node->args->len > arg_len)
                {
                    error_at(token->str, "Only up to %d function arguments are supported.", arg_len);
                }
            }

            return node;
        }

        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar)
        {
            node->offset = lvar->offset;
        }
        else
        {
            error_at(tok->str, "The variable is not defined.");
        }
        return node;
    }

    if (consume(D_INT))
    {
        Type *type = calloc(1, sizeof(Type));
        type->ty = INT;

        while (consume("*"))
        {
            Type *ptr_typ = calloc(1, sizeof(Type));
            ptr_typ->ty = PTR;
            ptr_typ->ptr_to = type;
            type = ptr_typ;
        }

        Token *variable_tok = consume_ident();
        if (!variable_tok)
            error_at(variable_tok->str, "Expected itentifier.");

        LVar *lvar = new_lvar(variable_tok, type);

        Node *node;
        node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->type = lvar->type;
        node->offset = lvar->offset;

        vec_push(lvars, lvar);

        return node;
    }

    // そうでなければ数値のはず
    return new_node_num(expect_number());
}
