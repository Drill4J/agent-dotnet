#include "pch.h"
#include "CorProfilerInfo.h"
#include "CProfilerCallback.h"

// This file contains concrete implementations of factory-like functions to create instances of `ICoreInteract` from `CorProfilerInfo` class
// It can be replaced to alternative implementatons of `ICoreInteract`, for example in Unit tests project.

namespace Drill4dotNet
{
    template<typename TQueryInterface, typename TLogger>
    std::unique_ptr<ICoreInteract> CreateCorProfilerInfo(TQueryInterface query, const TLogger logger)
    {
        return std::unique_ptr<ICoreInteract>{ new CorProfilerInfo(query, logger) };
    }

    template
        std::unique_ptr<ICoreInteract> CreateCorProfilerInfo<IUnknown*, LogToProClient>(
            IUnknown* pICorProfilerInfoUnk,
            const LogToProClient logger);

    template<typename TQueryInterface, typename TLogger>
    std::unique_ptr<ICoreInteract> TryCreateCorProfilerInfo(TQueryInterface query, const TLogger logger)
    {
        if (const auto oCorProfilerInfo = CorProfilerInfo<TLogger>::TryCreate(query, logger);
            oCorProfilerInfo.has_value())
        {
            return std::unique_ptr<ICoreInteract>{ new CorProfilerInfo(std::move(oCorProfilerInfo.value())) };
        }
        return nullptr;
    }

    template
        std::unique_ptr<ICoreInteract> TryCreateCorProfilerInfo<IUnknown*, LogToProClient>(
            IUnknown* pICorProfilerInfoUnk,
            const LogToProClient logger);
}
