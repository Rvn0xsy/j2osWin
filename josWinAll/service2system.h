#pragma once

#if defined(_M_AMD64)
#include "efs/efs-x64.h"
#else
#include "efs/efs-win32.h"
#endif
#include "shellcode.h"
#include <iostream>
#include <userenv.h>
#include <tchar.h>
#include <strsafe.h>
#include <sddl.h>
#include <thread>
#include <string>

void GetSystemAsImpersonatedUser(HANDLE hToken);

void StartNamedPipeAndGetSystem();

void ExecuteCommand(PWCHAR Command);

void Service2System();

