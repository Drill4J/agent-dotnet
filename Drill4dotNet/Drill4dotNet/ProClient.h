#pragma once

#include "framework.h"
#include <string>
#include <sstream>
#include <optional>
#include "LogBuffer.h"
#include "FunctionInfo.h"

namespace Drill4dotNet
{
    struct FunctionMetaInfo
    {
        std::wstring name;
        //TODO: std::wstring className;
        //TODO: std::wstring fullName;
    };

    typedef std::map<FunctionID, FunctionMetaInfo> TFunctionInfoMap;
    typedef std::map<FunctionID, long> TFunctionCountMap;

    class ProClient
    {
    public:
        ProClient();
        ~ProClient();
        LogBuffer<std::wostream> Log() const;
        std::wistream& Key();

        void MapFunctionInfo(const FunctionID& id, const FunctionMetaInfo& info) noexcept;
        std::optional<FunctionMetaInfo> GetFunctionInfo(const FunctionID& id) const noexcept;
        void FunctionCalled(const FunctionID& id) noexcept;
        const TFunctionInfoMap& GetMapOfFunctionNames() const
        {
            return m_functionNames;
        }
        const TFunctionCountMap& GetMapOfFunctionCounts() const
        {
            return m_functionCounts;
        }

    protected:
        std::wostream& m_ostream;
        std::wistream& m_istream;
        TFunctionInfoMap  m_functionNames;
        TFunctionCountMap m_functionCounts;
    };
}
