#include "pch.h"
#include "CDrillProfiler.h"
#include "CProfilerCallback.h"

namespace Drill4dotNet
{

    CDrillProfiler::CDrillProfiler()
        : CProfilerCallback(dynamic_cast<ProClient&>(*this))
    {
    }

    HRESULT CDrillProfiler::FinalConstruct()
    {
        Log() << L"Entering CDrillProfiler::FinalConstruct";
        return S_OK;
    }

    void CDrillProfiler::FinalRelease()
    {
        Log() << L"Entering CDrillProfiler::FinalRelease";
    }

}

// Attach definitions of IID_ICorProfilerCallback, IID_ICorProfilerCallback2, IID_ICorProfilerInfo2, IID_ICorProfilerInfo3, etc.
#include <corprof_i.cpp>
