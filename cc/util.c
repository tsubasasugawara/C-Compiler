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
    if (type->ty == PTR)
        return 8;
    if (type->ty == INT)
        return 4;

    assert(type->ty == ARRAY);
    return size_of(type->ptr_to) * type->array_size;
}
