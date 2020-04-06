#include "HackerMultiThreadDevice.h"

#include <log.h>
#include <PointerSet.h>

ThreadSafePointerSet HackerMultithread::m_List;

// Todo: Might need to flesh this out with stereo texture and all too, if we ever see these
// multithread variants.

HackerMultithread::HackerMultithread(ID3D10Multithread* pDevice)
	: IDirect3DUnknown((IUnknown*)pDevice)
{

}

HackerMultithread* HackerMultithread::GetDirect3DMultithread(ID3D10Multithread* pOrig)
{
	HackerMultithread* p = (HackerMultithread*) m_List.GetDataPtr(pOrig);
	if (!p)
	{
		p = new HackerMultithread(pOrig);
		if (pOrig) m_List.AddMember(pOrig, p);
	}
	return p;
}

STDMETHODIMP_(ULONG) HackerMultithread::AddRef(THIS)
{
	m_pUnk->AddRef();
	return ++m_ulRef;
}

STDMETHODIMP_(ULONG) HackerMultithread::Release(THIS)
{
	LogDebug("ID3D10Multithread::Release handle=%p, counter=%d\n", m_pUnk, m_ulRef);

	m_pUnk->Release();

	ULONG ulRef = --m_ulRef;

	if (ulRef <= 0)
	{
		LogDebug("  deleting self\n");

		if (m_pUnk) m_List.DeleteMember(m_pUnk); m_pUnk = 0;
		delete this;
		return 0L;
	}
	return ulRef;
}

STDMETHODIMP_(void) HackerMultithread::Enter(THIS)
{
	LogDebug("ID3D10Multithread::Enter called\n");

	GetD3D10MultithreadDevice()->Enter();
}

STDMETHODIMP_(void) HackerMultithread::Leave(THIS)
{
	LogDebug("ID3D10Multithread::Leave called\n");

	GetD3D10MultithreadDevice()->Leave();
}

STDMETHODIMP_(BOOL) HackerMultithread::SetMultithreadProtected(THIS_
	/* [annotation] */
	__in  BOOL bMTProtect)
{
	LogDebug("ID3D10Multithread::SetMultithreadProtected called with bMTProtect = %d\n", bMTProtect);

	BOOL ret = GetD3D10MultithreadDevice()->SetMultithreadProtected(bMTProtect);
	LogDebug("  returns %d\n", ret);

	return ret;
}

STDMETHODIMP_(BOOL) HackerMultithread::GetMultithreadProtected(THIS)
{
	LogDebug("ID3D10Multithread::GetMultithreadProtected called\n");

	BOOL ret = GetD3D10MultithreadDevice()->GetMultithreadProtected();
	LogDebug("  returns %d\n", ret);

	return ret;
}