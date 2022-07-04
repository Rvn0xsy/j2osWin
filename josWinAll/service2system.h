#pragma once

#if defined(_M_AMD64)
#include "efs/efs-x64.h"
#else
#include "efs/efs-win32.h"
#endif

#include <iostream>
#include <userenv.h>
#include <tchar.h>
#include <strsafe.h>
#include <sddl.h>
#include <thread>
#include <string>

typedef enum _PROCESS_INFORMATION_CLASS_NT {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    MaxProcessInfoClass
} PROCESS_INFORMATION_CLASS_NT, * PPROCESS_INFORMATION_CLASS_NT;

typedef struct _PROCESS_ACCESS_TOKEN {
	HANDLE                  Token;
	HANDLE                  Thread;
} PROCESS_ACCESS_TOKEN, * PPROCESS_ACCESS_TOKEN;

typedef ULONG(__stdcall* pfn_NtSetInformationProcess)(
    IN HANDLE               ProcessHandle,
    IN PROCESS_INFORMATION_CLASS_NT ProcessInformationClass,
    IN PVOID                ProcessInformation,
    IN ULONG                ProcessInformationLength);

void GetSystemAsImpersonatedUser(HANDLE hToken);

void StartNamedPipeAndGetSystem();

void ConnectEvilPipe();

void Service2System(PWCHAR Command);
