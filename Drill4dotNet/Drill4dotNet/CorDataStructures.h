#pragma once

#include "framework.h"
#include <string>

namespace Drill4dotNet
{
    struct FunctionName
    {
        std::wstring ownName;
        std::wstring className;
    };

    struct FunctionInfoWithoutName
    {
        ClassID classId;
        ModuleID moduleId;
        mdToken token;
    };

    struct FunctionInfo : public FunctionInfoWithoutName
    {
        FunctionName name;
        std::wstring fullName() const
        {
            return name.className + L"." + name.ownName;
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

    struct ClassInfoWithoutName
    {
        ModuleID moduleId;
        mdTypeDef typeDefToken;
        CorTypeAttr corTypeAttributes;
    };

    struct ClassInfo : public ClassInfoWithoutName
    {
        std::wstring name;
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

    struct AssemblyProps
    {
        std::wstring Name;
        DWORD Flags;
    };

    struct TypeDefProps
    {
        std::wstring Name;
        DWORD Flags;
        mdToken Extends;
    };

    struct MethodProps
    {
        mdTypeDef EnclosingClass;
        std::wstring Name;
        DWORD Attributes;
        ULONG CodeRelativeVirtualAddress;
        DWORD ImplementationFlags;
    };
}
