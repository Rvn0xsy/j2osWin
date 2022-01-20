#include <Windows.h>
#include <stdio.h>  

int main(int argc, char* argv[]) {
	PCHAR DllName = NULL;
	PCHAR ExportFunctionName = NULL;
	PCHAR ExportFunctionName_ANSI = NULL;
	HMODULE hModule = NULL;
	typedef int(__stdcall* TestFunction)(VOID);
	TestFunction CallFunction;
	switch (argc)
	{
	case 2:
		DllName = argv[1];
		break;
	case 3:
		DllName = argv[1];
		ExportFunctionName = argv[2];
		break;
	default:
		printf("[*] Usage: %s <DLL Name> [Export Function Name] \n", argv[0]);
		printf("[*] Example: \n--> TeDll.exe calc.dll\n--> TeDll.exe Lib.dll GetMessage");
		return 0;
	}
	// 加载DLL文件
	hModule = LoadLibraryA(DllName);
	if (hModule == NULL) {
		wprintf(L"[+] LoadLibrary Failed! Code: %d \n", GetLastError());
		return GetLastError();
	}
	Sleep(3000);
	if (ExportFunctionName != NULL) {
		CallFunction = (TestFunction)GetProcAddress(hModule, ExportFunctionName);
		CallFunction();
	}
	printf("[+] Please press down any key to end this program.\n");
	getchar();
}