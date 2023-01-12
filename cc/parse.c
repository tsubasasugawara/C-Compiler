#include "./cc.h"

// ローカル変数
Vector *lvars;

// TODO: 関数をマップで管理する

Type int_ty = {TY_INT, NULL, 0};

Type *new_type(TypeKind ty)
{
    Type *type = calloc(1, sizeof(Type));
    type->ty = ty;
    return type;
}

// 宣言されている型を返す
Type *parse_type()
{
    if (consume(D_INT))
    {
        return new_type(TY_INT);
    }
    return NULL;
}

// *を繰り返し読み、ポインタ型を実現する
Type *parse_pointer_type(Type *type)
{
    while (consume("*"))
    {
        Type *ptr_typ = new_type(TY_PTR);
        ptr_typ->ptr_to = type;
        type = ptr_typ;
    }
    return type;
}

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

LVar *new_lvar(Token *tok, Type *type)
{
    LVar *lvar;
    lvar = calloc(1, sizeof(LVar));
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->type = type;

    int prev_offset = 0;
    if (lvars->len > 0)
    {
        LVar *last = vec_last(lvars);
        prev_offset = last->offset;
    }
    if (type->ty == TY_ARRAY)
        lvar->offset = prev_offset + size_of(type);
    else
        lvar->offset = prev_offset + 8;

    return lvar;
}

Function *new_function(char *func_name, Node *node, Vector *lvars)
{
    Function *func = calloc(1, sizeof(Function));
    func->name = func_name;
    func->node = node;
    func->lvars = lvars;
    return func;
}

Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_node_binop(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = new_node(ND_NUM);
    node->val = val;
    node->type = &int_ty;
    return node;
}

Node *new_node_lvar(int offset, Type *type)
{
    Node *node = new_node(ND_LVAR);
    node->offset = offset;
    node->type = type;
    return node;
}

Node *new_node_function(char *func_name, Node *stmt, Vector *params)
{
    Node *node = new_node(ND_FUNC);
    node->name = func_name;
    node->body = stmt;
    node->params = params;
    return node;
}

Node *new_call_func(Token *tok)
{
    if (consume("("))
    {
        Node *node = new_node(ND_CALL);
        node->name = strndup(tok->str, tok->len);
        node->args = new_vec();
        // TODO:関数に合わせて変える
        node->type = &int_ty;

        while (!consume(")"))
        {
            consume(",");
            Node *arg = expr();
            vec_push(node->args, arg);

            size_t args_len = get_register_list_length();
            if (node->args->len > args_len)
            {
                error_at(token->str, "Only up to %d function arguments are supported.", args_len);
            }
        }

        return node;
    }
    return NULL;
}

Node *new_call_array(LVar *lvar)
{
    if (consume("["))
    {
        Node *index = expr();
        Node *array = new_node_lvar(lvar->offset, lvar->type);
        expect("]");

        Node *array_offset = new_node_binop(ND_ADD, array, index);
        Node *array_access = new_node(ND_DEREF);
        array_access->lhs = array_offset;
        array_access->type = array_offset->lhs->type->ptr_to;
        return array_access;
    }
    return NULL;
}

Node *new_vardef(Type *var_type)
{
    Type *type = new_type(var_type->ty);
    type = parse_pointer_type(type);

    Token *variable_tok = consume_ident();
    if (!variable_tok)
        error_at(variable_tok->str, "Expected itentifier.");

    if (consume("["))
    {
        int array_len = expect_number();
        if (!array_len)
            error_at(token->str, "Specify the length of the array.");

        Type *array_type = new_type(TY_ARRAY);
        array_type->array_size = array_len;
        array_type->ptr_to = type;
        type = array_type;
        expect("]");
    }

    LVar *lvar = new_lvar(variable_tok, type);
    vec_push(lvars, lvar);

    return new_node_lvar(lvar->offset, lvar->type);
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
    Program *program = calloc(1, sizeof(Program));
    program->funcs = new_vec();

    while (!at_eof())
    {
        Type *return_type = parse_type();
        if (!return_type)
            error("Specify the return type.");

        Token *tok = consume_ident();
        if (!tok)
            error_at(tok->str, "A top-level function definition is required.");

        Vector *params = new_vec();
        lvars = new_vec();

        expect("(");
        while (!consume(")"))
        {
            Type *arg_type = parse_type();
            if (!arg_type)
                error("Define type.");

            Token *param_tok = consume_ident();
            if (!param_tok || param_tok->kind != TK_IDENT)
                error_at(param_tok->str, "Arguments required.");

            size_t args_len = get_register_list_length();
            if (params->len > args_len)
                error_at(param_tok->str, "Only up to %d function arguments are supported.", args_len);

            vec_push(lvars, new_lvar(param_tok, arg_type));
            vec_push(params, new_node_lvar((params->len + 1) * 8, arg_type));
            consume(",");
        }

        char *func_name = strndup(tok->str, tok->len);
        Node *node = new_node_function(func_name, stmt(), params);
        node->return_type = return_type;
        vec_push(program->funcs, new_function(node->name, node, lvars));
    }

    return program;
}

Node *stmt()
{
    Node *node;

    if (consume(D_RETURN))
    {
        node = new_node(ND_RETURN);
        node->lhs = expr();
        expect(";");
    }
    else if (consume(D_FOR))
    {
        node = new_node(ND_FOR);
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
        node = new_node(ND_WHILE);
        expect("(");
        node->condition = expr();
        expect(")");
        node->body = stmt();
    }
    else if (consume(D_IF))
    {
        node = new_node(ND_IF);
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
        node = new_node(ND_BLOCK);
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
        node = new_node_binop(ND_ASSIGN, node, assign());
    return node;
}

Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume("=="))
            node = new_node_binop(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node_binop(ND_NE, node, relational());
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
            node = new_node_binop(ND_LT, node, add());
        else if (consume("<="))
            node = new_node_binop(ND_LE, node, add());
        else if (consume(">"))
            node = new_node_binop(ND_LT, add(), node);
        else if (consume(">="))
            node = new_node_binop(ND_LE, add(), node);
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
        {
            node = new_node_binop(ND_ADD, node, mul());
            if (node->lhs->type->ty != TY_PTR && node->rhs->type->ty == TY_PTR)
            {
                swap_node(&node->lhs, &node->rhs);
                assert(node->lhs->type->ty == TY_PTR);
            }
            node->type = node->lhs->type;
        }
        else if (consume("-"))
        {
            node = new_node_binop(ND_SUB, node, mul());
            node->type = node->lhs->type;
        }
        else
        {
            return node;
        }
    }
}

Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
        {
            node = new_node_binop(ND_MUL, node, unary());
            node->type = node->lhs->type;
        }
        else if (consume("/"))
        {
            node = new_node_binop(ND_DIV, node, unary());
            node->type = node->lhs->type;
        }
        else
            return node;
    }
}

Node *unary()
{
    if (consume("+"))
    {
        return primary();
    }
    else if (consume("-"))
    {
        Node *node = new_node_binop(ND_SUB, new_node_num(0), primary());
        node->type = node->lhs->type;
        return node;
    }
    else if (consume("&"))
    {
        Node *node = new_node_binop(ND_ADDR, unary(), NULL);
        node->type = node->lhs->type;
        return node;
    }
    else if (consume("*"))
    {
        Node *node = new_node_binop(ND_DEREF, unary(), NULL);
        node->type = node->lhs->type;
        return node;
    }
    else if (consume(D_SIZEOF))
    {
        Node *node = unary();
        return new_node_num(size_of(node->type));
    }
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
        Node *node = new_call_func(tok);
        if (node)
            return node;

        LVar *lvar = find_lvar(tok);
        if (!lvar)
            error_at(tok->str, "The variable is not defined.");

        node = new_call_array(lvar);
        if (node)
            return node;

        return new_node_lvar(lvar->offset, lvar->type);
    }

    Type *var_type = parse_type();
    if (var_type)
        return new_vardef(var_type);

    // そうでなければ数値のはず
    return new_node_num(expect_number());
}
