#include "pch.h"
#include "CDrillProfiler.h"
#include "CProfilerCallback.h"

namespace Drill4dotNet
{

    CDrillProfiler::CDrillProfiler()
        : CProfilerCallback(dynamic_cast<ProClient&>(*this))
        , ProClient(IConnector::CreateInstance())
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
