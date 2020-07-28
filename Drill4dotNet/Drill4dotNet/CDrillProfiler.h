#pragma once

#include "Drill4dotNet_i.h"
#include "CProfilerCallback.h"
#include "ProClient.h"
#include "CorProfilerInfo.h"
#include "MetaDataDispenser.h"
#include "ConnectorImplementation.h"

namespace Drill4dotNet
{
    using TConnector = Connector<std::function<std::vector<AstEntity>()>, std::function<void(const PackagesPrefixes&)>>;
    class ATL_NO_VTABLE CDrillProfiler
        : public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>
        , public ATL::CComCoClass<CDrillProfiler, &CLSID_DrillProfiler>
        , public CProfilerCallback<
            TConnector,
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
