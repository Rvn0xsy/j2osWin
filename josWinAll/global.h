#pragma once

#define PIPE_NAME L"\\\\.\\pipe\\josPipe"
#define BUFF_SIZE 1024
#define METHOD_WMI_CREATE_PROCESS 0x00 // WMIC ��������
#define METHOD_SHELL_CODE_LOADE 0x01  // SHELLCODE ����
#define METHOD_MINIDUMP_LSASS 0x02   // ת��Lsass.exe
#define METHOD_ADD_USER 0x03  // ����û�
#define METHOD_GETSYSTEM 0x04  // ����EFS��ȡSYSTEM��Token
#define METHOD_SYSTEM_EXECUTE 0x05 // ��SYSTEMȨ��ִ������
#define METHOD_OPEN_SYSTEM_DOOR 0x06