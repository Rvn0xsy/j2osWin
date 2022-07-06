#pragma once
#include <windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <TlHelp32.h>
#pragma comment( lib, "Dbghelp.lib" )
#define _CRT_SECURE_NO_WARNINGS

typedef HRESULT(WINAPI* _MiniDumpW)(DWORD arg1, DWORD arg2, PWCHAR cmdline);

BOOL CheckPrivilege();

BOOL EnableDebugPrivilege();

DWORD GetLsassPID();

BOOL CheckFileExists(PWCHAR file);


BOOL MiniDumpLsass();