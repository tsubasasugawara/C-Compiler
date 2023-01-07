#include "./cc.h"

Token *token;
char *user_input;
Program *program;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("The number of arguments is incorrect.");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    program = parse();

    codegen();

    return 0;
}
