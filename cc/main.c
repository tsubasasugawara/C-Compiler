#include "./cc.h"

Token *token;
Program *program;

char *file_path;
char *source;

char *read_file(char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
        error("cannot open %s: %s", path, strerror(errno));

    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    if (size == 0 || buf[size - 1] != '\n')
        buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("The number of arguments is incorrect.");
        return 1;
    }

    file_path = argv[1];
    char *source = read_file(file_path);
    token = tokenize(source);
    program = parse();

    codegen();

    return 0;
}
