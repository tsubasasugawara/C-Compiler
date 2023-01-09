#include "./cc.h"

void swap_node(Node **p, Node **q)
{
    Node *r = *p;
    *p = *q;
    *q = r;
    return;
}
