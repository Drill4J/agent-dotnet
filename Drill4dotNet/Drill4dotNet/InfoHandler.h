#pragma once

#include "framework.h"
#include <string>
#include <optional>
#include <unordered_map>
#include "LogBuffer.h"

namespace Drill4dotNet
{
    struct FunctionMetaInfo
    {
        std::wstring name;
    };
    struct FunctionRuntimeInfo
    {
        unsigned long callCount = 0UL;
    };

    using TFunctionMetaInfoMap = std::unordered_map<FunctionID, FunctionMetaInfo>;
    using TFunctionRuntimeInfoMap = std::unordered_map<FunctionID, FunctionRuntimeInfo>;

    class InfoHandler
    {
    public:
        explicit InfoHandler(std::wostream& log);

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
        using Logger = LogBuffer<std::wostream>;
        Logger Log() const;
        std::wostream& m_ostream;
        TFunctionMetaInfoMap m_functionNames;
        TFunctionRuntimeInfoMap m_functionCounts;
    };
}
