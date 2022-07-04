#include "utils.h"


wchar_t* char2wchar(const char* cchar)
{
    wchar_t* m_wchar;//�������ָ��
    //ȡ���ֳ���
    int len = MultiByteToWideChar(
        CP_ACP, //������
        0, //��־
        cchar, //�����ַ���
        strlen(cchar),//�����ַ������� 
        NULL,//�����ַ��� 
        0);//�����ַ�������
    m_wchar = new wchar_t[len + 1];//Ϊ����ָ������ڴ�
    ZeroMemory(m_wchar, len + 1);
    MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);//���ƶ��ֵ�����
    m_wchar[len] = '\0';//�ַ�����β
    return m_wchar;//����ָ��
}