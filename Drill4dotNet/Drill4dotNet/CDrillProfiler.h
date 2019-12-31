#pragma once
#include "Drill4dotNet_i.h"
#include "CProfilerCallback.h"
#include "ProClient.h"

namespace Drill4dotNet
{
    class ATL_NO_VTABLE CDrillProfiler
        : public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>
        , public ATL::CComCoClass<CDrillProfiler, &CLSID_DrillProfiler>
        , ProClient
        , public CProfilerCallback
    {
    public:
        CDrillProfiler();

        BEGIN_COM_MAP(CDrillProfiler)
            COM_INTERFACE_ENTRY(ICorProfilerCallback)
            COM_INTERFACE_ENTRY(ICorProfilerCallback2)
        END_COM_MAP()
        DECLARE_PROTECT_FINAL_CONSTRUCT()

        HRESULT FinalConstruct();
        void FinalRelease();
    };

    // TODO: OBJECT_ENTRY_AUTO(__uuidof(DrillProfiler), CDrillProfiler)
}
