#include "pch.h"
#include "CDrillProfiler.h"
#include "CProfilerCallback.h"

namespace Drill4dotNet
{

    CDrillProfiler::CDrillProfiler()
        : CProfilerCallback(*this)
    {
    }

    HRESULT CDrillProfiler::FinalConstruct()
    {
        return S_OK;
    }

    void CDrillProfiler::FinalRelease()
    {
    }

}
