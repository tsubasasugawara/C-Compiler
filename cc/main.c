#include "./cc.h"

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
    Node *node = expr();

    // �A�Z���u���̑O���������o��
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // ���ۍ\���؂�����Ȃ���R�[�h����
    gen(node);

    // �X�^�b�N�g�b�v�Ɏ��S�̂̒l���c���Ă���͂��Ȃ̂�
    // �����RAX�Ƀ��[�h���Ċ֐�����̕Ԃ�l�Ƃ���
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
