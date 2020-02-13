#pragma once

#include "framework.h"
#include <string>
#include <optional>
#include <unordered_map>
#include "LogBuffer.h"
#include "CorDataStructures.h"

namespace Drill4dotNet
{
    struct FunctionRuntimeInfo
    {
        unsigned long callCount = 0UL;
    };

    using TAppDomainInfoMap = std::unordered_map<AppDomainID, AppDomainInfo>;
    using TAssemblyInfoMap = std::unordered_map<AssemblyID, AssemblyInfo>;
    using TModuleInfoMap = std::unordered_map<ModuleID, ModuleInfo>;
    using TClassInfoMap = std::unordered_map<ClassID, ClassInfo>;
    using TFunctionInfoMap = std::unordered_map<FunctionID, FunctionInfo>;
    using TFunctionRuntimeInfoMap = std::unordered_map<FunctionID, FunctionRuntimeInfo>;

    class InfoHandler
    {
    public:
        explicit InfoHandler(std::wostream& log);

        void OutputStatistics() const;
        void MapFunctionInfo(const FunctionID id, const FunctionInfo& info) noexcept;
        std::optional<FunctionInfo> TryGetFunctionInfo(const FunctionID id) const noexcept;
        void FunctionCalled(const FunctionID id) noexcept;
        void MapAppDomainInfo(const AppDomainID id, const AppDomainInfo& info) noexcept;
        std::optional<AppDomainInfo> TryGetAppDomainInfo(const AppDomainID id) const noexcept;
        void OutputAppDomainInfo(const AppDomainID id) const;
        void MapAssemblyInfo(const AssemblyID id, const AssemblyInfo& info) noexcept;
        std::optional<AssemblyInfo> TryGetAssemblyInfo(const AssemblyID id) const noexcept;
        void OutputAssemblyInfo(const AssemblyID id) const;
        void MapModuleInfo(const ModuleID id, const ModuleInfo& info) noexcept;
        std::optional<ModuleInfo> TryGetModuleInfo(const ModuleID id) const noexcept;
        void OutputModuleInfo(const ModuleID id) const;
        void MapClassInfo(const ClassID id, const ClassInfo& info) noexcept;
        std::optional<ClassInfo> TryGetClassInfo(const ClassID id) const noexcept;
        void OutputClassInfo(const ClassID id) const;
    protected:
        using Logger = LogBuffer<std::wostream>;
        Logger Log() const;
        std::wostream& m_ostream;
        TFunctionInfoMap m_functionInfos;
        TFunctionRuntimeInfoMap m_functionCounts;
        TAppDomainInfoMap m_appDomainInfos;
        TAssemblyInfoMap m_assemblyInfos;
        TModuleInfoMap m_moduleInfos;
        TClassInfoMap m_classInfos;
    };
}
