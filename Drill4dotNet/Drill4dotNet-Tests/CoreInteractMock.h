#pragma once

#include <optional>
#include <functional>
#include <gmock/gmock.h>

#include "CorDataStructures.h"
#include "ComWrapperBase.h"
#include "MetadataImportMock.h"

namespace Drill4dotNet
{
    class CoreInteractMock
    {
    private:
        inline static std::function<void(CoreInteractMock&, IUnknown*, TrivialLogger)> m_onCreate{};

        class OnCreateRaii
        {
        public:
            OnCreateRaii(std::function<void(CoreInteractMock&, IUnknown*, TrivialLogger)> onCreate)
            {
                CoreInteractMock::m_onCreate = onCreate;
            }

            ~OnCreateRaii()
            {
                CoreInteractMock::m_onCreate = decltype(CoreInteractMock::m_onCreate){};
            }

            OnCreateRaii(const OnCreateRaii&) = delete;
            OnCreateRaii& operator=(const OnCreateRaii&) & = delete;

            OnCreateRaii(OnCreateRaii&&) = default;
            OnCreateRaii& operator=(OnCreateRaii&&) & = default;
        };

    public:
        static auto SetCreationCallBack(std::function<void(CoreInteractMock&, IUnknown*, TrivialLogger)> onCreate)
        {
            return OnCreateRaii(onCreate);
        }

        template<typename TQueryInterface, typename TLogger>
        CoreInteractMock(TQueryInterface query, const TLogger logger)
        {
            static_assert(std::is_same_v<TQueryInterface, IUnknown*>);
            static_assert(std::is_same_v<TLogger, TrivialLogger>);

            if (m_onCreate)
            {
                m_onCreate(*this, query, logger);
            }
        }

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
        MOCK_METHOD(std::optional<ClassInfoWithoutName>, TryGetClassInfo, (const ClassID classId), (const));
        MOCK_METHOD(FunctionInfoWithoutName, GetFunctionInfo, (const FunctionID functionId), (const));
        MOCK_METHOD(std::optional<FunctionInfoWithoutName>, TryGetFunctionInfo, (const FunctionID functionId), (const));
        MOCK_METHOD(std::vector<std::byte>, GetMethodIntermediateLanguageBody, (const FunctionInfo& functionInfo), (const));
        MOCK_METHOD(std::optional<std::vector<std::byte>>, TryGetMethodIntermediateLanguageBody, (const FunctionInfo& functionInfo), (const));
        MOCK_METHOD(void, SetILFunctionBody, (const FunctionInfo& target, const std::vector<std::byte>& newILMethodBody), (const));
        MOCK_METHOD(bool, TrySetILFunctionBody, (const FunctionInfo& target, const std::vector<std::byte>& newILMethodBody), (const));
        MOCK_METHOD(ClassInfoWithoutName, GetClassInfo, (const ClassID classId), (const));

        MetadataImportMock GetModuleMetadata(const ModuleID moduleId, const TrivialLogger logger) const
        {
            return { logger };
        }

        std::optional<MetadataImportMock> TryGetModuleMetadata(const ModuleID moduleId, const TrivialLogger logger) const
        {
            return { logger };
        }
    };
}
