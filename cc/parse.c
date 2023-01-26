#include "./cc.h"

// ローカル変数
Vector *lvars;

Vector *funcs;

Map *gvars;

Type int_ty = {TY_INT, NULL, 0};

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

Type *new_type(TypeKind ty)
{
    Type *type = calloc(1, sizeof(Type));
    type->ty = ty;
    return type;
}

// 宣言されている型を返す
Type *parse_type()
{
    Type *type;
    if (consume(D_INT))
        type = new_type(TY_INT);
    else
        return NULL;

    while (consume("*"))
    {
        Type *ptr_typ = new_type(TY_PTR);
        ptr_typ->ptr_to = type;
        type = ptr_typ;
    }

    return type;
}

Var *find_lvar(Token *tok)
{
    for (int i = 0; i < lvars->len; i++)
    {
        Var *var;
        var = lvars->data[i];
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    }
    return NULL;
}

Var *find_gvar(Token *tok)
{
    return map_get(gvars, tok->str);
}

Var *new_var(Token *tok, Type *type, bool is_local)
{
    Var *var;
    var = calloc(1, sizeof(Var));
    var->name = strndup(tok->str, tok->len);
    var->len = tok->len;
    var->type = type;

    int prev_offset = 0;
    if (is_local && lvars->len > 0)
    {
        Var *last = vec_last(lvars);
        prev_offset = last->offset;
    }
    var->offset = prev_offset + calc_need_byte(var->type);

    return var;
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

Node *new_node_gvar(char *name, Type *type)
{
    Node *node = new_node(ND_GVAR);
    node->name = strdup(name);
    node->type = type;
    return node;
}

Node *new_node_var(Token *tok)
{
    Node *node = calloc(1, sizeof(Node));

    Var *lvar = find_lvar(tok);
    if (lvar)
        return new_node_lvar(lvar->offset, lvar->type);

    Var *var = find_gvar(tok);
    if (!var)
        error_at(tok->str, "The variable is not defined.");
    return new_node_gvar(var->name, var->type);
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

Node *new_call_array(Node *node, Node *index)
{
    Node *array_offset = new_node_binop(ND_ADD, node, index);
    Node *array_access = new_node(ND_DEREF);
    array_access->lhs = array_offset;
    array_access->type = array_offset->lhs->type->ptr_to;
    node = array_access;
}

Node *new_vardef(Type *type)
{
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

    Var *lvar = new_var(variable_tok, type, true);
    vec_push(lvars, lvar);

    return new_node_lvar(lvar->offset, lvar->type);
}

Program *parse()
{
    Program *program = calloc(1, sizeof(Program));
    funcs = new_vec();
    gvars = new_map();

    while (!at_eof())
    {
        Type *type = parse_type();
        if (!type)
            error("Specify the return type.");

        Token *tok = consume_ident();
        if (!tok)
            error_at(tok->str, "A top-level function definition is required.");

        if (consume("("))
        {
            Vector *params = new_vec();
            lvars = new_vec();

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

                vec_push(lvars, new_var(param_tok, arg_type, true));
                vec_push(params, new_node_lvar((params->len + 1) * 8, arg_type));
                consume(",");
            }

            char *func_name = strndup(tok->str, tok->len);
            Node *node = new_node_function(func_name, stmt(), params);
            node->return_type = type;
            vec_push(funcs, new_function(node->name, node, lvars));
        }
        else
        {
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
            Var *var = new_var(tok, type, false);
            map_put(gvars, var->name, var);
            expect(";");
        }
    }
    program->funcs = funcs;
    program->gvars = gvars;

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

        node = new_node_var(tok);

        if (consume("["))
        {
            Node *index = expr();
            expect("]");
            node = new_call_array(node, index);
        }

        return node;
    }

    Type *var_type = parse_type();
    if (var_type)
        return new_vardef(var_type);

    // そうでなければ数値のはず
    return new_node_num(expect_number());
}
