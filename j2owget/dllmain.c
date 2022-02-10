// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <Urlmon.h>
typedef int(__stdcall* URLDownloadToFileFunction)(LPUNKNOWN pCaller, LPCTSTR szURL, LPCTSTR szFileName, DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB);

VOID DownLoad(char* url, char* dest) {
/*
    Library Urlmon.lib
    DLL Urlmon.dll
    HRESULT URLDownloadToFile(
             LPUNKNOWN            pCaller,
             LPCTSTR              szURL,
             LPCTSTR              szFileName,
  _Reserved_ DWORD                dwReserved,
             LPBINDSTATUSCALLBACK lpfnCB
);
*/
    HMODULE hUrlmon =  LoadLibraryA("Urlmon.dll");
    if (hUrlmon != NULL) {
        URLDownloadToFileFunction URLDownloadToFileProc = (URLDownloadToFileFunction)GetProcAddress(hUrlmon, "URLDownloadToFile");
        URLDownloadToFileProc(NULL, url, dest, 0, NULL);
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
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

