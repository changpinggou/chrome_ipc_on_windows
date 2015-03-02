// dllmain.h : Declaration of module class.

class CbaseModule : public ATL::CAtlDllModuleT< CbaseModule >
{
public :
	DECLARE_LIBID(LIBID_baseLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_BASE, "{DF15D5BD-D8E2-4E39-A329-19EF7D49C4A9}")
};

extern class CbaseModule _AtlModule;
