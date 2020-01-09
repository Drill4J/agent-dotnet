// dllmain.cpp : Implementation of DllMain.

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "Drill4dotNet_i.h"
#include "dllmain.h"
#include <iostream>

CDrill4dotNetModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (DLL_PROCESS_ATTACH == dwReason)
    {
        DisableThreadLibraryCalls(hInstance);
    }
    std::wcout << L"DllMain(" << hInstance << L", " << dwReason << L")\n";
    return _AtlModule.DllMain(dwReason, lpReserved);
}
