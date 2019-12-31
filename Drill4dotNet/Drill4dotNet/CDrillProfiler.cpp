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
        this->Log() << L"Entering CDrillProfiler::FinalConstruct\n";
        return S_OK;
    }

    void CDrillProfiler::FinalRelease()
    {
        this->Log() << L"Entering CDrillProfiler::FinalRelease\n";
    }

}
