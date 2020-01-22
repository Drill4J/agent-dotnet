#pragma once

#include "framework.h"
#include <string>
#include <sstream>
#include <optional>
#include <unordered_map>
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
    struct FunctionRuntimeInfo
    {
        unsigned long callCount = 0UL;
    };

    using TFunctionMetaInfoMap = std::unordered_map<FunctionID, FunctionMetaInfo>;
    using TFunctionRuntimeInfoMap = std::unordered_map<FunctionID, FunctionRuntimeInfo>;

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
        const TFunctionMetaInfoMap& GetFunctionMetaInfoMap() const
        {
            return m_functionNames;
        }
        const TFunctionRuntimeInfoMap& GetFunctionRuntimeInfoMap() const
        {
            return m_functionCounts;
        }

    protected:
        std::wostream& m_ostream;
        std::wistream& m_istream;
        TFunctionMetaInfoMap m_functionNames;
        TFunctionRuntimeInfoMap m_functionCounts;
    };
}
