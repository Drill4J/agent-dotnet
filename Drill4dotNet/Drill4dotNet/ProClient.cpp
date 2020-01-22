#include "pch.h"
#include "ProClient.h"

using namespace std;

namespace Drill4dotNet
{
    ProClient::ProClient()
        : m_ostream(wcout)
        , m_istream(wcin)
    {
    }

    ProClient::~ProClient()
    {
    }

    LogBuffer<std::wostream> ProClient::Log() const
    {
        return LogBuffer<std::wostream>(m_ostream);
    }

    wistream& ProClient::Key()
    {
        return m_istream;
    }

    void ProClient::MapFunctionInfo(const FunctionID& id, const FunctionMetaInfo& info) noexcept
    {
        try
        {
            // allow to overwrite function information on secondary mapping (TBD?)
            m_functionNames[id] = info;
            m_functionCounts[id] = 0;
        }
        catch (const std::exception & ex)
        {
            Log() << "ProClient::MapFunctionInfo: exception while inserting function info by id [" << id << "]. " << ex.what();
        }
    }

    std::optional<FunctionMetaInfo> ProClient::GetFunctionInfo(const FunctionID& id) const noexcept
    {
        try
        {
            TFunctionInfoMap::const_iterator it = m_functionNames.find(id);
            if (m_functionNames.end() != it)
            {
                return it->second;
            }
            else
            {
                Log() << "ProClient::GetFunctionInfo: cannot find function by id [" << id << "].";
            }
        }
        catch (const std::exception & ex)
        {
            Log() << "ProClient::GetFunctionInfo: exception while accessing function info by id [" << id << "]. " << ex.what();
        }
        return std::nullopt;
    }

    void ProClient::FunctionCalled(const FunctionID& id) noexcept
    {
        try
        {
            TFunctionCountMap::iterator it = m_functionCounts.find(id);
            if (m_functionCounts.end() != it)
            {
                it->second += 1;
            }
            else
            {
                Log() << "ProClient::FunctionCalled: cannot find function by id [" << id << "].";
            }
        }
        catch (const std::exception & ex)
        {
            Log() << "ProClient::FunctionCalled: exception while accessing function info by id [" << id << "]. " << ex.what();
        }
    }

}
