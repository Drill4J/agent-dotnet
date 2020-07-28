#include "pch.h"
#include "CDrillProfiler.h"
#include "CProfilerCallback.h"

namespace Drill4dotNet
{

    CDrillProfiler::CDrillProfiler()
        : CProfilerCallback(ConsoleLogger{})
    {
    }

    HRESULT CDrillProfiler::FinalConstruct()
    {
        m_logger.Log() << L"Entering CDrillProfiler::FinalConstruct";
        return S_OK;
    }

    void CDrillProfiler::FinalRelease()
    {
        m_logger.Log() << L"Entering CDrillProfiler::FinalRelease";
    }

}
