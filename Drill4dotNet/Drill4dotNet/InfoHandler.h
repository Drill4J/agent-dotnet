#pragma once

#include "framework.h"
#include <string>
#include <optional>
#include <unordered_map>
#include "LogBuffer.h"
#include "CorDataStructures.h"
#include <filesystem>

namespace Drill4dotNet
{
    extern std::filesystem::path s_Drill4dotNetLibFilePath;

    struct FunctionRuntimeInfo
    {
        unsigned long callCount = 0UL;
    };

    struct InjectionMetaData
    {
        mdAssembly  Assembly = 0;
        mdTypeDef   Class = 0;
        mdMethodDef Function = 0;
    };

    template <IsLogger Logger>
    class InfoHandler
    {
    private:
        using TAppDomainInfoMap = std::unordered_map<AppDomainID, AppDomainInfo>;
        using TAssemblyInfoMap = std::unordered_map<AssemblyID, AssemblyInfo>;
        using TModuleInfoMap = std::unordered_map<ModuleID, ModuleInfo>;
        using TClassInfoMap = std::unordered_map<ClassID, ClassInfo>;
        using TFunctionInfoMap = std::unordered_map<FunctionID, FunctionInfo>;
        using TFunctionRuntimeInfoMap = std::unordered_map<FunctionID, FunctionRuntimeInfo>;

        TFunctionInfoMap m_functionInfos;
        TFunctionRuntimeInfoMap m_functionCounts;
        TAppDomainInfoMap m_appDomainInfos;
        TAssemblyInfoMap m_assemblyInfos;
        TModuleInfoMap m_moduleInfos;
        TClassInfoMap m_classInfos;
        InjectionMetaData m_injectionMetaData;
        Logger m_logger;

    public:
        explicit InfoHandler(Logger logger)
            : m_logger{ logger }
        {
        }

        void OutputStatistics() const
        {
            m_logger.Log() << L"Statistics:";
            m_logger.Log() << L"Total number of functions mapped: " << m_functionInfos.size();

            size_t countFunctionsCalled = std::count_if(
                m_functionCounts.cbegin(),
                m_functionCounts.cend(),
                [](const auto& info) -> bool
            {
                return info.second.callCount > 0;
            });
            m_logger.Log() << L"Total number of functions called: " << countFunctionsCalled;
        }

        void MapFunctionInfo(const FunctionID id, const FunctionInfo& info) noexcept
        {
            try
            {
                m_functionInfos[id] = info;
                m_functionCounts[id] = { 0 };
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::MapFunctionInfo: exception while inserting function info by id [" << id << "]. " << ex.what();
            }
        }

        std::optional<FunctionInfo> TryGetFunctionInfo(const FunctionID id) const noexcept
        {
            try
            {
                if (const auto it = m_functionInfos.find(id);
                    m_functionInfos.end() != it)
                {
                    return it->second;
                }
                else
                {
                    m_logger.Log() << "InfoHandler::TryGetFunctionInfo: cannot find function by id [" << id << "].";
                }
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::TryGetFunctionInfo: exception while accessing function info by id [" << id << "]. " << ex.what();
            }
            return std::nullopt;
        }

        void FunctionCalled(const FunctionID id) noexcept
        {
            try
            {
                if (auto it = m_functionCounts.find(id);
                    m_functionCounts.end() != it)
                {
                    it->second.callCount += 1;
                }
                else
                {
                    m_logger.Log() << "InfoHandler::FunctionCalled: cannot find function by id [" << id << "].";
                }
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::FunctionCalled: exception while accessing function info by id [" << id << "]. " << ex.what();
            }
        }

        void MapAppDomainInfo(const AppDomainID id, const AppDomainInfo& info) noexcept
        {
            try
            {
                m_appDomainInfos[id] = info;
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::MapAppDomainInfo: exception while inserting AppDomain info by id [" << id << "]. " << ex.what();
            }
        }

        std::optional<AppDomainInfo> TryGetAppDomainInfo(const AppDomainID id) const noexcept
        {
            try
            {
                if (const auto it = m_appDomainInfos.find(id);
                    m_appDomainInfos.end() != it)
                {
                    return it->second;
                }
                else
                {
                    m_logger.Log() << "InfoHandler::TryGetAppDomainInfo: cannot find App Domain info by id [" << id << "].";
                }
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::TryGetAppDomainInfo: exception while accessing App Domain info by id [" << id << "]. " << ex.what();
            }
            return std::nullopt;
        }

        void OutputAppDomainInfo(const AppDomainID id) const
        {
            if (const auto it = m_appDomainInfos.find(id);
                m_appDomainInfos.end() != it)
            {
                m_logger.Log() << L"Domain name: " << it->second.name << L", process id: " << it->second.processId;
            }
        }

        void MapAssemblyInfo(const AssemblyID id, const AssemblyInfo& info) noexcept
        {
            try
            {
                m_assemblyInfos[id] = info;
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::MapAssemblyInfo: exception while inserting Assembly info by id [" << id << "]. " << ex.what();
            }
        }

        std::optional<AssemblyInfo> TryGetAssemblyInfo(const AssemblyID id) const noexcept
        {
            try
            {
                if (const auto it = m_assemblyInfos.find(id);
                    m_assemblyInfos.end() != it)
                {
                    return it->second;
                }
                else
                {
                    m_logger.Log() << "InfoHandler::TryGetAssemblyInfo: cannot find Assembly info by id [" << id << "].";
                }
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::TryGetAssemblyInfo: exception while accessing Assembly info by id [" << id << "]. " << ex.what();
            }
            return std::nullopt;
        }

        void OutputAssemblyInfo(const AssemblyID id) const
        {
            if (const auto _assembly = m_assemblyInfos.find(id);
                m_assemblyInfos.end() != _assembly)
            {
                const auto _domain = m_appDomainInfos.find(_assembly->second.appDomainId);
                const auto _module = m_moduleInfos.find(_assembly->second.moduleId);
                m_logger.Log()
                    << L"Assembly name: " << _assembly->second.name
                    << L", its app domain: " << _assembly->second.appDomainId
                    << L" (" << (m_appDomainInfos.end() != _domain ? _domain->second.name : L"<unknown>") << L")"
                    << L", its module: " << _assembly->second.moduleId
                    << L" (" << (m_moduleInfos.end() != _module ? _module->second.name : L"<unknown>") << L")"
                    ;
            }
        }

        void MapModuleInfo(const ModuleID id, const ModuleInfo& info) noexcept
        {
            try
            {
                m_moduleInfos[id] = info;
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::MapModuleInfo: exception while inserting Module info by id [" << id << "]. " << ex.what();
            }
        }

        std::optional<ModuleInfo> TryGetModuleInfo(const ModuleID id) const noexcept
        {
            try
            {
                if (const auto it = m_moduleInfos.find(id);
                    m_moduleInfos.end() != it)
                {
                    return it->second;
                }
                else
                {
                    m_logger.Log() << "InfoHandler::TryGetModuleInfo: cannot find Module info by id [" << id << "].";
                }
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::TryGetModuleInfo: exception while accessing Module info by id [" << id << "]. " << ex.what();
            }
            return std::nullopt;
        }

        void OutputModuleInfo(const ModuleID id) const
        {
            if (const auto _module = m_moduleInfos.find(id);
                m_moduleInfos.end() != _module)
            {
                const auto _assembly = m_assemblyInfos.find(_module->second.assemblyId);
                m_logger.Log()
                    << L"Module name: " << _module->second.name
                    << L", loaded by address: " << HexOutput(_module->second.baseLoadAddress)
                    << L", its assembly: " << _module->second.assemblyId
                    << L" (" << (m_assemblyInfos.end() != _assembly ? _assembly->second.name : L"<unknown>") << L")"
                    ;
            }
        }

        void MapClassInfo(const ClassID id, const ClassInfo& info) noexcept
        {
            try
            {
                m_classInfos[id] = info;
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::MapClassInfo: exception while inserting Class info by id [" << id << "]. " << ex.what();
            }
        }

        std::optional<ClassInfo> TryGetClassInfo(const ClassID id) const noexcept
        {
            try
            {
                if (const auto it = m_classInfos.find(id);
                    m_classInfos.end() != it)
                {
                    return it->second;
                }
                else
                {
                    m_logger.Log() << "InfoHandler::TryGetClassInfo: cannot find Class info by id [" << id << "].";
                }
            }
            catch (const std::exception& ex)
            {
                m_logger.Log() << "InfoHandler::TryGetClassInfo: exception while accessing Class info by id [" << id << "]. " << ex.what();
            }
            return std::nullopt;
        }

        void OutputClassInfo(const ClassID id) const
        {
            if (const auto _class = m_classInfos.find(id);
                m_classInfos.end() != _class)
            {
                const auto _module = m_moduleInfos.find(_class->second.moduleId);
                m_logger.Log()
                    << L"Class/Type name: " << _class->second.name
                    << L", its module: " << _class->second.moduleId
                    << L" (" << (m_moduleInfos.end() != _module ? _module->second.name : L"<unknown>") << L")"
                    ;
            }
        }

        InjectionMetaData GetInjectionMetaData() const noexcept
        {
            return m_injectionMetaData;
        }

        void SetInjectionMetaData(const InjectionMetaData& injection) noexcept
        {
            m_injectionMetaData = injection;
        }
    };
}
