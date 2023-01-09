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
    switch (type->ty)
    {
    case INT:
        return 4;
    case PTR:
        return 8;
    }
    return 0;
}
