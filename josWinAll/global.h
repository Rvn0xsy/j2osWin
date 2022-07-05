#pragma once

#define PIPE_NAME L"\\\\.\\pipe\\josPipe"
#define BUFF_SIZE 1024
#define METHOD_WMI_CREATE_PROCESS 0x00 // WMIC 创建进程
#define METHOD_SHELL_CODE_LOADE 0x01  // SHELLCODE 加载
#define METHOD_MINIDUMP_LSASS 0x02   // 转储Lsass.exe
#define METHOD_ADD_USER 0x03  // 添加用户
#define METHOD_GETSYSTEM 0x04  // 利用EFS获取SYSTEM的Token
#define METHOD_SYSTEM_EXECUTE 0x05 // 以SYSTEM权限执行命令
#define METHOD_OPEN_SYSTEM_DOOR 0x06