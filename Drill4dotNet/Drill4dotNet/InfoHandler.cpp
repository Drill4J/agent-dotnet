#include "pch.h"
#include "InfoHandler.h"
#include "ProClient.h"
#include "CorDataStructures.h"
#include <algorithm>

namespace Drill4dotNet
{
    InfoHandler::InfoHandler(std::wostream& log)
        : m_ostream(log)
    {
    }

    InfoHandler::Logger InfoHandler::Log() const
    {
        return LogBuffer<std::wostream>(m_ostream);
    }

    void InfoHandler::MapFunctionInfo(const FunctionID& id, const FunctionMetaInfo& info) noexcept
    {
        try
        {
            // allow to overwrite function information on secondary mapping (TBD?)
            m_functionNames[id] = info;
            m_functionCounts[id] = { 0 };
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::MapFunctionInfo: exception while inserting function info by id [" << id << "]. " << ex.what();
        }
    }

    std::optional<FunctionMetaInfo> InfoHandler::GetFunctionInfo(const FunctionID& id) const noexcept
    {
        try
        {
            if (TFunctionMetaInfoMap::const_iterator it = m_functionNames.find(id);
                m_functionNames.end() != it)
            {
                return it->second;
            }
            else
            {
                Log() << "InfoHandler::GetFunctionInfo: cannot find function by id [" << id << "].";
            }
        }
        catch (const std::exception & ex)
        {
            Log() << "InfoHandler::GetFunctionInfo: exception while accessing function info by id [" << id << "]. " << ex.what();
        }
        return std::nullopt;
    }

    void InfoHandler::FunctionCalled(const FunctionID& id) noexcept
    {
        try
        {
            if (TFunctionRuntimeInfoMap::iterator it = m_functionCounts.find(id);
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
        Log() << L"Total number of functions mapped: " << m_functionNames.size();

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
}
