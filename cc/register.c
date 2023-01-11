#include "./cc.h"

char *register_list[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

size_t get_register_list_length()
{
    return sizeof(register_list) / sizeof(register_list[0]);
}

char *get_register_name(int num)
{
    int len = get_register_list_length();
    if (num >= len)
        error("Only up to %d function arguments are supported.", len);
    if (len <= 0)
        error("Register not defined");

    char *register_name = strndup(register_list[num], sizeof(register_list[num]));

    return register_name;
}
