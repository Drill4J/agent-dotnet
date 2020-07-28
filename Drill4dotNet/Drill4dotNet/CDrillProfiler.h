#pragma once

#include "Drill4dotNet_i.h"
#include "CProfilerCallback.h"
#include "ProClient.h"
#include "CorProfilerInfo.h"
#include "MetaDataDispenser.h"
#include "Connector.h"

namespace Drill4dotNet
{
    class ATL_NO_VTABLE CDrillProfiler
        : public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>
        , public ATL::CComCoClass<CDrillProfiler, &CLSID_DrillProfiler>
        , public CProfilerCallback<
            Connector,
            CorProfilerInfo<ConsoleLogger>,
            MetaDataDispenser<ConsoleLogger>,
            MetaDataAssemblyImport<ConsoleLogger>,
            MetaDataImport<ConsoleLogger>,
        ConsoleLogger>
    {
    public:
        CDrillProfiler();

        DECLARE_REGISTRY_RESOURCEID(IDR_DRILL4DOTNET)
        BEGIN_COM_MAP(CDrillProfiler)
            COM_INTERFACE_ENTRY(ICorProfilerCallback)
            COM_INTERFACE_ENTRY(ICorProfilerCallback2)
        END_COM_MAP()
        DECLARE_PROTECT_FINAL_CONSTRUCT()

        HRESULT FinalConstruct();
        void FinalRelease();
    };

    OBJECT_ENTRY_AUTO(__uuidof(DrillProfiler), CDrillProfiler)
}
