#include "pch.h"
#include "InfoHandler.h"
#include "ProClient.h"
#include "FunctionInfo.h"

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
            TFunctionMetaInfoMap::const_iterator it = m_functionNames.find(id);
            if (m_functionNames.end() != it)
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
            TFunctionRuntimeInfoMap::iterator it = m_functionCounts.find(id);
            if (m_functionCounts.end() != it)
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
}
