// dllmain.cpp : Implementation of DllMain.

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "Drill4dotNet_i.h"
#include "dllmain.h"
#include "LogBuffer.h"
#include <iostream>

CDrill4dotNetModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (DLL_PROCESS_ATTACH == dwReason)
    {
        DisableThreadLibraryCalls(hInstance);
    }
    Drill4dotNet::LogStdout() << L"DllMain(" << hInstance << L", " << dwReason << L")";

    return _AtlModule.DllMain(dwReason, lpReserved);
}
