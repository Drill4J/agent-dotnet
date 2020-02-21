#pragma once

#include "CoreInteract.h"
#include <gmock/gmock.h>

namespace Drill4dotNet
{
    class CoreInteractMock : public ICoreInteract
    {
    public:
        MOCK_METHOD(RuntimeInformation, GetRuntimeInformation, (), (const));
        MOCK_METHOD(std::optional<RuntimeInformation>, TryGetRuntimeInformation, (), (const));
        MOCK_METHOD(void, SetEventMask, (const uint32_t eventMask), (const));
        MOCK_METHOD(bool, TrySetEventMask, (const uint32_t eventMask), (const));
        MOCK_METHOD(void, SetFunctionIDMapper, (FunctionIDMapper* pFunc), (const));
        MOCK_METHOD(bool, TrySetFunctionIDMapper, (FunctionIDMapper* pFunc), (const));
        MOCK_METHOD(void, SetEnterLeaveFunctionHooks, (FunctionEnter2* pFuncEnter, FunctionLeave2* pFuncLeave, FunctionTailcall2* pFuncTailcall), (const));
        MOCK_METHOD(bool, TrySetEnterLeaveFunctionHooks, (FunctionEnter2* pFuncEnter, FunctionLeave2* pFuncLeave, FunctionTailcall2* pFuncTailcall), (const));
        MOCK_METHOD(AppDomainInfo, GetAppDomainInfo, (const AppDomainID appDomainId), (const));
        MOCK_METHOD(std::optional<AppDomainInfo>, TryGetAppDomainInfo, (const AppDomainID appDomainId), (const));
        MOCK_METHOD(AssemblyInfo, GetAssemblyInfo, (const AssemblyID assemblyId), (const));
        MOCK_METHOD(std::optional<AssemblyInfo>, TryGetAssemblyInfo, (const AssemblyID assemblyId), (const));
        MOCK_METHOD(ModuleInfo, GetModuleInfo, (const ModuleID moduleId), (const));
        MOCK_METHOD(std::optional<ModuleInfo>, TryGetModuleInfo, (const ModuleID moduleId), (const));
        MOCK_METHOD(std::optional<ClassInfo>, TryGetClassInfo, (const ClassID classId), (const));
        MOCK_METHOD(FunctionInfo, GetFunctionInfo, (const FunctionID functionId), (const));
        MOCK_METHOD(std::optional<FunctionInfo>, TryGetFunctionInfo, (const FunctionID functionId), (const));
        MOCK_METHOD(std::vector<std::byte>, GetMethodIntermediateLanguageBody, (const FunctionInfo& functionInfo), (const));
        MOCK_METHOD(std::optional<std::vector<std::byte>>, TryGetMethodIntermediateLanguageBody, (const FunctionInfo& functionInfo), (const));
        MOCK_METHOD(void, SetILFunctionBody, (const FunctionInfo& target, const std::vector<std::byte>& newILMethodBody), (const));
        MOCK_METHOD(bool, TrySetILFunctionBody, (const FunctionInfo& target, const std::vector<std::byte>& newILMethodBody), (const));
        MOCK_METHOD(ClassInfo, GetClassInfo, (const ClassID classId), (const));
    };
}
