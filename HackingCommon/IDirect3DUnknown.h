#pragma once

#include <Unknwn.h>

class IDirect3DUnknown : public IUnknown
{
public:
	IUnknown* m_pUnk;
	ULONG m_ulRef;
	
	IDirect3DUnknown(IUnknown* pDevice) :
		m_pUnk(pDevice)
	{}

	virtual STDMETHODIMP QueryInterface(THIS_ REFIID riid, void** ppvObj);
};