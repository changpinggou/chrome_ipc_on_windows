// dllmain.h : Declaration of module class.

class CcommonModule : public ATL::CAtlDllModuleT< CcommonModule >
{
public :
	DECLARE_LIBID(LIBID_commonLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_COMMON, "{F67CAC45-0AF9-4BB2-913C-5E149EA30A0B}")
};

extern class CcommonModule _AtlModule;
