#include "./cc.h"

Token *token;
char *user_input;
Node *code[100];

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("The number of arguments is incorrect.");
        return 1;
    }

    // �g�[�N�i�C�Y���ăp�[�X����
    user_input = argv[1];
    token = tokenize();
    program();

    // �A�Z���u���̑O���������o��
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // �v�����[�O
    // �ϐ�26���̗̈���m�ۂ���
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    // �擪�̎����珇�ɃR�[�h����
    for (int i = 0; code[i]; i++)
    {
        gen(code[i]);

        // ���̕]�����ʂƂ��ăX�^�b�N�Ɉ�̒l���c���Ă���
        // �͂��Ȃ̂ŁA�X�^�b�N�����Ȃ��悤�Ƀ|�b�v���Ă���
        printf("    pop rax\n");
    }

    // �G�s���[�O
    // �Ō�̎��̌��ʂ�RAX�Ɏc���Ă���̂ł��ꂪ�Ԃ�l�ɂȂ�
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}
