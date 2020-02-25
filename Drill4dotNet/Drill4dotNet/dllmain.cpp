// dllmain.cpp : Implementation of DllMain.

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "Drill4dotNet_i.h"
#include "dllmain.h"
#include "LogBuffer.h"
#include "OutputUtils.h"
#include "InfoHandler.h"
#include <iostream>

CDrill4dotNetModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    Drill4dotNet::LogStdout() << L"DllMain(" << hInstance << L", " << dwReason << L")";
    if (DLL_PROCESS_ATTACH == dwReason)
    {
        DisableThreadLibraryCalls(hInstance);

        DWORD rSize = _MAX_PATH;
        std::wstring drill4dotNetModuleFileName(_MAX_PATH, L'\0');
        do
        {
            drill4dotNetModuleFileName.resize(rSize, L'\0');
            rSize = ::GetModuleFileName(hInstance, drill4dotNetModuleFileName.data(), static_cast<unsigned long>(drill4dotNetModuleFileName.size()));
            rSize *= 2;
        } while (::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
        Drill4dotNet::s_Drill4dotNetLibFilePath = Drill4dotNet::TrimTrailingNulls(drill4dotNetModuleFileName);
        Drill4dotNet::LogStdout() << Drill4dotNet::s_Drill4dotNetLibFilePath.wstring();
    }
    return _AtlModule.DllMain(dwReason, lpReserved);
}
