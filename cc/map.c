#include "./cc.h"

Map *new_map()
{
    Map *map = calloc(1, sizeof(Map));
    map->keys = new_vec();
    map->elems = new_vec();
}

void map_put(Map *map, void *key, void *elem)
{
    vec_push(map->keys, key);
    vec_push(map->elems, elem);
}

void *map_get(Map *map, void *key)
{
    for (int i = 0; i < map->keys->len; i++)
        if (map->keys->data[i] == key)
            return map->keys->data[i];
    return NULL;
}
