#include "utils.h"


wchar_t* char2wchar(const char* cchar)
{
    wchar_t* m_wchar;//定义宽字指针
    //取多字长度
    int len = MultiByteToWideChar(
        CP_ACP, //代码面
        0, //标志
        cchar, //多字字符串
        strlen(cchar),//多字字符串长度 
        NULL,//宽字字符串 
        0);//宽字字符串长度
    m_wchar = new wchar_t[len + 1];//为宽字指针分配内存
    ZeroMemory(m_wchar, len + 1);
    MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);//复制多字到宽字
    m_wchar[len] = '\0';//字符串结尾
    return m_wchar;//返回指针
}