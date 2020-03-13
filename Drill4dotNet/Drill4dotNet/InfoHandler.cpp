#include "pch.h"
#include "InfoHandler.h"
#include "ProClient.h"
#include "CorDataStructures.h"
#include "OutputUtils.h"
#include <algorithm>

namespace Drill4dotNet
{
    std::filesystem::path s_Drill4dotNetLibFilePath;

    InfoHandler::InfoHandler(std::wostream& log)
        : m_ostream(log)
    {
    }

    InfoHandler::Logger InfoHandler::Log() const
    {
        return LogBuffer<std::wostream>(m_ostream);
    }

    void InfoHandler::MapFunctionInfo(const FunctionID id, const FunctionInfo& info) noexcept
    {
        try
        {
            m_functionInfos[id] = info;
            m_functionCounts[id] = { 0 };
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::MapFunctionInfo: exception while inserting function info by id [" << id << "]. " << ex.what();
        }
    }

    std::optional<FunctionInfo> InfoHandler::TryGetFunctionInfo(const FunctionID id) const noexcept
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
                Log() << "InfoHandler::TryGetFunctionInfo: cannot find function by id [" << id << "].";
            }
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::TryGetFunctionInfo: exception while accessing function info by id [" << id << "]. " << ex.what();
        }
        return std::nullopt;
    }

    void InfoHandler::FunctionCalled(const FunctionID id) noexcept
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
                Log() << "InfoHandler::FunctionCalled: cannot find function by id [" << id << "].";
            }
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::FunctionCalled: exception while accessing function info by id [" << id << "]. " << ex.what();
        }
    }
    
    void InfoHandler::OutputStatistics() const
    {
        Log() << L"Statistics:";
        Log() << L"Total number of functions mapped: " << m_functionInfos.size();

        size_t countFunctionsCalled = std::count_if(
            m_functionCounts.cbegin(),
            m_functionCounts.cend(),
            [](const auto& info) -> bool
            {
                return info.second.callCount > 0;
            }
        );
        Log() << L"Total number of functions called: " << countFunctionsCalled;
    }

    void InfoHandler::MapAppDomainInfo(const AppDomainID id, const AppDomainInfo& info) noexcept
    {
        try
        {
            m_appDomainInfos[id] = info;
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::MapAppDomainInfo: exception while inserting AppDomain info by id [" << id << "]. " << ex.what();
        }
    }

    std::optional<AppDomainInfo> InfoHandler::TryGetAppDomainInfo(const AppDomainID id) const noexcept
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
                Log() << "InfoHandler::TryGetAppDomainInfo: cannot find App Domain info by id [" << id << "].";
            }
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::TryGetAppDomainInfo: exception while accessing App Domain info by id [" << id << "]. " << ex.what();
        }
        return std::nullopt;
    }

    void InfoHandler::OutputAppDomainInfo(const AppDomainID id) const
    {
        if (const auto it = m_appDomainInfos.find(id);
            m_appDomainInfos.end() != it)
        {
            Log() << L"Domain name: " << it->second.name << L", process id: " << it->second.processId;
        }
    }

    void InfoHandler::MapAssemblyInfo(const AssemblyID id, const AssemblyInfo& info) noexcept
    {
        try
        {
            m_assemblyInfos[id] = info;
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::MapAssemblyInfo: exception while inserting Assembly info by id [" << id << "]. " << ex.what();
        }
    }

    std::optional<AssemblyInfo> InfoHandler::TryGetAssemblyInfo(const AssemblyID id) const noexcept
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
                Log() << "InfoHandler::TryGetAssemblyInfo: cannot find Assembly info by id [" << id << "].";
            }
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::TryGetAssemblyInfo: exception while accessing Assembly info by id [" << id << "]. " << ex.what();
        }
        return std::nullopt;
    }

    void InfoHandler::OutputAssemblyInfo(const AssemblyID id) const
    {
        if (const auto _assembly = m_assemblyInfos.find(id);
            m_assemblyInfos.end() != _assembly)
        {
            const auto _domain = m_appDomainInfos.find(_assembly->second.appDomainId);
            const auto _module = m_moduleInfos.find(_assembly->second.moduleId);
            Log()
                << L"Assembly name: " << _assembly->second.name
                << L", its app domain: " << _assembly->second.appDomainId
                << L" (" << (m_appDomainInfos.end() != _domain ? _domain->second.name : L"<unknown>") << L")"
                << L", its module: " << _assembly->second.moduleId
                << L" (" << (m_moduleInfos.end() != _module ? _module->second.name : L"<unknown>") << L")"
                ;
        }
    }

    void InfoHandler::MapModuleInfo(const ModuleID id, const ModuleInfo& info) noexcept
    {
        try
        {
            m_moduleInfos[id] = info;
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::MapModuleInfo: exception while inserting Module info by id [" << id << "]. " << ex.what();
        }
    }

    std::optional<ModuleInfo> InfoHandler::TryGetModuleInfo(const ModuleID id) const noexcept
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
                Log() << "InfoHandler::TryGetModuleInfo: cannot find Module info by id [" << id << "].";
            }
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::TryGetModuleInfo: exception while accessing Module info by id [" << id << "]. " << ex.what();
        }
        return std::nullopt;
    }

    void InfoHandler::OutputModuleInfo(const ModuleID id) const
    {
        if (const auto _module = m_moduleInfos.find(id);
            m_moduleInfos.end() != _module)
        {
            const auto _assembly = m_assemblyInfos.find(_module->second.assemblyId);
            Log()
                << L"Module name: " << _module->second.name
                << L", loaded by address: " << HexOutput(_module->second.baseLoadAddress)
                << L", its assembly: " << _module->second.assemblyId
                << L" (" << (m_assemblyInfos.end() != _assembly ? _assembly->second.name : L"<unknown>") << L")"
                ;
        }
    }

    void InfoHandler::MapClassInfo(const ClassID id, const ClassInfo& info) noexcept
    {
        try
        {
            m_classInfos[id] = info;
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::MapClassInfo: exception while inserting Class info by id [" << id << "]. " << ex.what();
        }
    }

    std::optional<ClassInfo> InfoHandler::TryGetClassInfo(const ClassID id) const noexcept
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
                Log() << "InfoHandler::TryGetClassInfo: cannot find Class info by id [" << id << "].";
            }
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::TryGetClassInfo: exception while accessing Class info by id [" << id << "]. " << ex.what();
        }
        return std::nullopt;
    }

    void InfoHandler::OutputClassInfo(const ClassID id) const
    {
        if (const auto _class = m_classInfos.find(id);
            m_classInfos.end() != _class)
        {
            const auto _module = m_moduleInfos.find(_class->second.moduleId);
            Log()
                << L"Class/Type name: " << _class->second.name
                << L", its module: " << _class->second.moduleId
                << L" (" << (m_moduleInfos.end() != _module ? _module->second.name : L"<unknown>") << L")"
                ;
        }
    }

    InjectionMetaData InfoHandler::GetInjectionMetaData() const noexcept
    {
        return m_injectionMetaData;
    }

    void InfoHandler::SetInjectionMetaData(const InjectionMetaData& injection) noexcept
    {
        m_injectionMetaData = injection;
    }
}
