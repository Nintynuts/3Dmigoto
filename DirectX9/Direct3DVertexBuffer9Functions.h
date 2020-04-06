
HackerVertexBuffer9::HackerVertexBuffer9(LPDIRECT3DVERTEXBUFFER9 pVertexBuffer)
    : IDirect3DUnknown((IUnknown*) pVertexBuffer),
	pendingCreateVertexBuffer(false),
	pendingLockUnlock(false),
	magic(0x7da43feb)
{
}

HackerVertexBuffer9* HackerVertexBuffer9::GetDirect3DVertexBuffer9(LPDIRECT3DVERTEXBUFFER9 pVertexBuffer)
{
    HackerVertexBuffer9* p = (HackerVertexBuffer9*) m_List.GetDataPtr(pVertexBuffer);
    if (!p)
    {
        p = new HackerVertexBuffer9(pVertexBuffer);
        if (pVertexBuffer) m_List.AddMember(pVertexBuffer, p);
    }
    return p;
}

STDMETHODIMP_(ULONG) HackerVertexBuffer9::AddRef(THIS)
{
	++m_ulRef;
	return m_pUnk->AddRef();
}

STDMETHODIMP_(ULONG) HackerVertexBuffer9::Release(THIS)
{
	LogDebug("HackerVertexBuffer9::Release handle=%x, counter=%d, this=%x\n", m_pUnk, m_ulRef, this);
	
    ULONG ulRef = m_pUnk ? m_pUnk->Release() : 0;
	LogDebug("  internal counter = %d\n", ulRef);
	
	--m_ulRef;

    if (ulRef == 0)
    {
		if (!gLogDebug) LogInfo("HackerVertexBuffer9::Release handle=%x, counter=%d, internal counter = %d\n", m_pUnk, m_ulRef, ulRef);
		LogInfo("  deleting self\n");
		
        if (m_pUnk) m_List.DeleteMember(m_pUnk); 
		m_pUnk = 0;
        delete this;
        return 0L;
    }
    return ulRef;
}

STDMETHODIMP HackerVertexBuffer9::GetDevice(THIS_ HackerDevice9** ppDevice)
{
	LogDebug("HackerVertexBuffer9::GetDevice called\n");
	
	CheckVertexBuffer9(this);
	IDirect3DDevice9 *origDevice;
	HRESULT hr = GetD3DVertexBuffer9()->GetDevice(&origDevice);
	if (hr != S_OK)
	{
		LogInfo("  failed with hr = %x\n", hr);
		
		return hr;
	}
	IDirect3DDevice9Ex *origDeviceEx;
	const IID IID_HackerDevice9Ex = { 0xb18b10ce, 0x2649, 0x405a, { 0x87, 0xf, 0x95, 0xf7, 0x77, 0xd4, 0x31, 0x3a } };
	hr = origDevice->QueryInterface(IID_HackerDevice9Ex, (void **) &origDeviceEx);
	origDevice->Release();
	if (hr != S_OK)
	{
		LogInfo("  failed IID_HackerDevice9Ex cast with hr = %x\n", hr);
		
		return hr;
	}
	*ppDevice = HackerDevice9::GetDirect3DDevice(origDeviceEx);
	return hr;
}

STDMETHODIMP HackerVertexBuffer9::SetPrivateData(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags)
{
	LogDebug("HackerVertexBuffer9::SetPrivateData called\n");
	
	CheckVertexBuffer9(this);
	return GetD3DVertexBuffer9()->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

STDMETHODIMP HackerVertexBuffer9::GetPrivateData(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData)
{
	LogDebug("HackerVertexBuffer9::GetPrivateData called\n");
	
	CheckVertexBuffer9(this);
	return GetD3DVertexBuffer9()->GetPrivateData(refguid, pData, pSizeOfData);
}

STDMETHODIMP HackerVertexBuffer9::FreePrivateData(THIS_ REFGUID refguid)
{
	LogDebug("HackerVertexBuffer9::GetPrivateData called\n");
	
	CheckVertexBuffer9(this);
	return GetD3DVertexBuffer9()->FreePrivateData(refguid);
}

STDMETHODIMP_(DWORD) HackerVertexBuffer9::SetPriority(THIS_ DWORD PriorityNew)
{
	LogDebug("HackerVertexBuffer9::SetPriority called\n");
	
	CheckVertexBuffer9(this);
	return GetD3DVertexBuffer9()->SetPriority(PriorityNew);
}

STDMETHODIMP_(DWORD) HackerVertexBuffer9::GetPriority(THIS)
{
	LogDebug("HackerVertexBuffer9::GetPriority called\n");
	
	CheckVertexBuffer9(this);
	return GetD3DVertexBuffer9()->GetPriority();
}

STDMETHODIMP_(void) HackerVertexBuffer9::PreLoad(THIS)
{
	LogDebug("HackerVertexBuffer9::GetPriority called\n");
	
	CheckVertexBuffer9(this);
	return GetD3DVertexBuffer9()->PreLoad();
}

STDMETHODIMP_(D3DRESOURCETYPE) HackerVertexBuffer9::GetType(THIS)
{
	LogDebug("HackerVertexBuffer9::GetType called\n");
	
	CheckVertexBuffer9(this);
	D3DRESOURCETYPE hr = GetD3DVertexBuffer9()->GetType();
	LogDebug("  returns ResourceType=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerVertexBuffer9::Lock(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags)
{
	LogDebug("HackerVertexBuffer9::Lock called with OffsetToLock=%d, SizeToLock=%d, Flags=%x\n", OffsetToLock, SizeToLock, Flags);
	

	if (!GetD3DVertexBuffer9())
	{
		if (!gLogDebug) LogInfo("HackerVertexBuffer9::Lock called\n");
		LogInfo("  postponing call because vertex buffer was not created yet.\n");
		
		if (!pendingLockUnlock)
		{
			_Flags = Flags;
			_Buffer = new char[_Length];
			pendingLockUnlock = true;
		}
		if (ppbData) *ppbData = _Buffer+OffsetToLock;
		return S_OK;
	}

	HRESULT hr = GetD3DVertexBuffer9()->Lock(OffsetToLock, SizeToLock, ppbData, Flags);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerVertexBuffer9::Unlock(THIS)
{
	LogDebug("HackerVertexBuffer9::Unlock called\n");
	

	if (!GetD3DVertexBuffer9())
	{
		if (!gLogDebug) LogInfo("HackerVertexBuffer9::Unlock called\n");
		LogInfo("  postponing call because vertex buffer was not created yet.\n");
		
		return S_OK;
	}

	CheckVertexBuffer9(this);
	HRESULT hr = GetD3DVertexBuffer9()->Unlock();
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerVertexBuffer9::GetDesc(THIS_ D3DVERTEXBUFFER_DESC *pDesc)
{
	LogDebug("HackerVertexBuffer9::GetDesc called\n");
	
	CheckVertexBuffer9(this);
	return GetD3DVertexBuffer9()->GetDesc(pDesc);
}
