#pragma once
#include <windows.h>
#include <string.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <Tchar.h>
#pragma comment(lib,"netapi32.lib")

BOOL CreateAdminUserInternal(void);