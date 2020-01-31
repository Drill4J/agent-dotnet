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
}