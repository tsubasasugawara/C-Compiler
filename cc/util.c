#include "./cc.h"

void swap_node(Node **p, Node **q)
{
    Node *r = *p;
    *p = *q;
    *q = r;
    return;
}

int size_of(Type *type)
{
    if (type->ty == TY_PTR)
        return 8;
    if (type->ty == TY_INT)
        return 4;

    assert(type->ty == TY_ARRAY);
    return size_of(type->ptr_to) * type->array_size;
}
