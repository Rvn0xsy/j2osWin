#pragma once
#include<Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#pragma comment(lib, "wbemuuid.lib")

int WMICCreateProcess(PWCHAR Command);