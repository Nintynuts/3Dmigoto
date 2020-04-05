
HackerSurface9::HackerSurface9(LPDIRECT3DSURFACE9 pSurface)
    : IDirect3DUnknown((IUnknown*) pSurface),
	pendingGetSurfaceLevel(false),
	magic(0x7da43feb)
{
}

HackerSurface9* HackerSurface9::GetDirect3DSurface9(LPDIRECT3DSURFACE9 pSurface)
{
    HackerSurface9* p = (HackerSurface9*) m_List.GetDataPtr(pSurface);
    if (!p)
    {
        p = new HackerSurface9(pSurface);
        if (pSurface) m_List.AddMember(pSurface, p);
    }
    return p;
}

STDMETHODIMP_(ULONG) HackerSurface9::AddRef(THIS)
{
	++m_ulRef;
	return m_pUnk->AddRef();
}

STDMETHODIMP_(ULONG) HackerSurface9::Release(THIS)
{
	LogDebug("HackerSurface9::Release handle=%x, counter=%d, this=%x\n", m_pUnk, m_ulRef, this);
	
    ULONG ulRef = m_pUnk ? m_pUnk->Release() : 0;
	LogDebug("  internal counter = %d\n", ulRef);
	
	--m_ulRef;

    if (ulRef == 0)
    {
		if (!gLogDebug) LogInfo("HackerSurface9::Release handle=%x, counter=%d, internal counter = %d\n", m_pUnk, m_ulRef, ulRef);
		LogInfo("  deleting self\n");
		
        if (m_pUnk) m_List.DeleteMember(m_pUnk); 
		m_pUnk = 0;
        delete this;
        return 0L;
    }
    return ulRef;
}

STDMETHODIMP HackerSurface9::GetDevice(THIS_ HackerDevice9** ppDevice)
{
	LogDebug("HackerSurface9::GetDevice called\n");
	
	CheckSurface9(this);
	IDirect3DDevice9 *origDevice;
	HRESULT hr = GetD3DSurface9()->GetDevice(&origDevice);
	if (hr != S_OK)
	{
		LogInfo("  failed with hr = %x\n", hr);
		
		if (ppDevice) *ppDevice = 0;
		return hr;
	}
	IDirect3DDevice9Ex *origDeviceEx;
	const IID IID_HackerDevice9Ex = { 0xb18b10ce, 0x2649, 0x405a, { 0x87, 0xf, 0x95, 0xf7, 0x77, 0xd4, 0x31, 0x3a } };
	hr = origDevice->QueryInterface(IID_HackerDevice9Ex, (void **) &origDeviceEx);
	origDevice->Release();
	if (hr != S_OK)
	{
		LogInfo("  failed IID_HackerDevice9Ex cast with hr = %x\n", hr);
		
		if (ppDevice) *ppDevice = 0;
		return hr;
	}
	*ppDevice = HackerDevice9::GetDirect3DDevice(origDeviceEx);
	return hr;
}

STDMETHODIMP HackerSurface9::SetPrivateData(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags)
{
	LogDebug("HackerSurface9::SetPrivateData called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

STDMETHODIMP HackerSurface9::GetPrivateData(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData)
{
	LogDebug("HackerSurface9::GetPrivateData called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->GetPrivateData(refguid, pData, pSizeOfData);
}

STDMETHODIMP HackerSurface9::FreePrivateData(THIS_ REFGUID refguid)
{
	LogDebug("HackerSurface9::GetPrivateData called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->FreePrivateData(refguid);
}

STDMETHODIMP_(DWORD) HackerSurface9::SetPriority(THIS_ DWORD PriorityNew)
{
	LogDebug("HackerSurface9::SetPriority called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->SetPriority(PriorityNew);
}

STDMETHODIMP_(DWORD) HackerSurface9::GetPriority(THIS)
{
	LogDebug("HackerSurface9::GetPriority called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->GetPriority();
}

STDMETHODIMP_(void) HackerSurface9::PreLoad(THIS)
{
	LogDebug("HackerSurface9::GetPriority called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->PreLoad();
}

STDMETHODIMP_(D3DRESOURCETYPE) HackerSurface9::GetType(THIS)
{
	LogDebug("HackerSurface9::GetType called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->GetType();
}

STDMETHODIMP HackerSurface9::GetContainer(THIS_ REFIID riid,void** ppContainer)
{
	LogDebug("HackerSurface9::GetContainer called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->GetContainer(riid, ppContainer);
}

STDMETHODIMP HackerSurface9::GetDesc(THIS_ D3DSURFACE_DESC *pDesc)
{
	LogDebug("HackerSurface9::GetDesc called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->GetDesc(pDesc);
}

STDMETHODIMP HackerSurface9::LockRect(THIS_ D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
{
	LogDebug("HackerSurface9::LockRect called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->LockRect(pLockedRect, pRect, Flags);
}

STDMETHODIMP HackerSurface9::UnlockRect(THIS)
{
	LogDebug("HackerSurface9::UnlockRect called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->UnlockRect();
}

STDMETHODIMP HackerSurface9::GetDC(THIS_ HDC *phdc)
{
	LogDebug("HackerSurface9::GetDC called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->GetDC(phdc);
}

STDMETHODIMP HackerSurface9::ReleaseDC(THIS_ HDC hdc)
{
	LogDebug("HackerSurface9::ReleaseDC called\n");
	
	CheckSurface9(this);
	return GetD3DSurface9()->ReleaseDC(hdc);
}
