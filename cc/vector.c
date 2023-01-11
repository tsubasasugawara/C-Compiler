#include "./cc.h"

Vector *new_vec()
{
    Vector *v = malloc(sizeof(Vector));
    v->data = malloc(sizeof(void *) * 16);
    v->capacity = 16;
    v->len = 0;
    return v;
}

void vec_push(Vector *v, void *elem)
{
    if (v->len == v->capacity)
    {
        v->capacity *= 2;
        v->data = realloc(v->data, sizeof(void *) * v->capacity);
    }

    v->data[v->len++] = elem;
}

void *vec_pop(Vector *v)
{
    assert(v->len);
    return v->data[--v->len];
}

void *vec_last(Vector *v)
{
    assert(v->len);
    return v->data[v->len - 1];
}
