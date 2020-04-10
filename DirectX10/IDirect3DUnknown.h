#pragma once
#include <Unknwn.h>

class IDirect3DUnknown
{
public:
	IUnknown* m_pRealUnk;
	IUnknown* m_pUnk;
	ULONG       m_ulRef;
	int m_dm_recordedCalls;

	IDirect3DUnknown(IUnknown* pUnk)
	{
		m_pUnk = pUnk;
		m_pRealUnk = pUnk;
		m_ulRef = 1;
		m_dm_recordedCalls = 0;
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	bool QueryInterface_DXGI_Callback(REFIID riid, void** ppvObj, HRESULT* result);
	IDirect3DUnknown* QueryInterface_Find_Wrapper(void* ppvObj);

	IUnknown* GetOrig()
	{
		return m_pUnk;
	}

	IUnknown* GetRealOrig()
	{
		return m_pRealUnk;
	}
};