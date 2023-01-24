#include "./cc.h"

Map *new_map()
{
    Map *map = calloc(1, sizeof(Map));
    map->keys = new_vec();
    map->elems = new_vec();
    return map;
}

/*
 * key�����݂��邩���m���߂�
 * @param map : �}�b�v�z��
 * @param key : �m�F�������L�[
 * @return : ���݂���΃C���f�b�N�X�A���݂��Ȃ����-1
 */
int map_find(Map *map, void *key)
{
    for (int i = 0; i < map->keys->len; i++)
        if (strcmp(map->keys->data[i], key))
            return i;
    return -1;
}

void map_put(Map *map, void *key, void *elem)
{
    if (map_find(map, key) != -1)
        return;

    vec_push(map->keys, key);
    vec_push(map->elems, elem);
}

void *map_get(Map *map, void *key)
{
    int index = map_find(map, key);
    if (index == -1)
        return NULL;

    return map->elems->data[index];
}
