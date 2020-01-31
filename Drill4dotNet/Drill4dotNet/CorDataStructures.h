#pragma once

#include "framework.h"
#include <string>

namespace Drill4dotNet
{
    struct FunctionInfo
    {
        ClassID classId;
        ModuleID moduleId;
        mdToken token;
    };
    struct FunctionName
    {
        std::wstring name;
        std::wstring className;
        std::wstring fullName() const
        {
            return className + L"." + name;
        }
    };
    struct AppDomainInfo
    {
        std::wstring name;
        ProcessID processId;
    };
    struct AssemblyInfo
    {
        std::wstring name;
        AppDomainID appDomainId;
        ModuleID moduleId;
    };
    struct ModuleInfo
    {
        std::wstring name;
        LPCBYTE baseLoadAddress;
        AssemblyID assemblyId;
    };
    struct ClassInfo
    {
        std::wstring name;
        ModuleID moduleId;
        mdTypeDef typeDefToken;
        CorTypeAttr corTypeAttributes;
    };
    struct RuntimeInformation
    {
        USHORT clrInstanceId;
        COR_PRF_RUNTIME_TYPE runtimeType;
        USHORT majorVersion;
        USHORT minorVersion;
        USHORT buildNumber;
        USHORT qfeVersion;
        std::wstring RuntimeType() const
        {
            switch (runtimeType)
            {
                case COR_PRF_DESKTOP_CLR: return L"Desktop CLR";
                case COR_PRF_CORE_CLR: return L"Core CLR";
                default: return L"Unknown CLR";
            }
        }
        std::wstring Version() const
        {
            std::wostringstream s;
            s << majorVersion << L"." << minorVersion << L"." << buildNumber;
            return s.str();
        }
        std::wstring QFEVersion() const
        {
            std::wostringstream s;
            s << qfeVersion;
            return s.str();
        }
    };
}
