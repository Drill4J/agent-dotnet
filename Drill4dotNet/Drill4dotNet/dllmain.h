// dllmain.h : Declaration of module class.

class CDrill4dotNetModule : public ATL::CAtlDllModuleT< CDrill4dotNetModule >
{
public:
    DECLARE_LIBID(LIBID_Drill4dotNetLib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_DRILL4DOTNET, "{bb54e37e-3e2e-4b45-ad99-223cb351098e}")
};

extern class CDrill4dotNetModule _AtlModule;
