
HackerIndexBuffer9::HackerIndexBuffer9(LPDIRECT3DINDEXBUFFER9 pIndexBuffer)
    : IDirect3DUnknown((IUnknown*) pIndexBuffer),
	pendingCreateIndexBuffer(false),
	pendingLockUnlock(false),
	magic(0x7da43feb)
{
}

HackerIndexBuffer9* HackerIndexBuffer9::GetDirect3DIndexBuffer9(LPDIRECT3DINDEXBUFFER9 pIndexBuffer)
{
    HackerIndexBuffer9* p = (HackerIndexBuffer9*) m_List.GetDataPtr(pIndexBuffer);
    if (!p)
    {
        p = new HackerIndexBuffer9(pIndexBuffer);
        if (pIndexBuffer) m_List.AddMember(pIndexBuffer, p);
    }
    return p;
}

STDMETHODIMP_(ULONG) HackerIndexBuffer9::AddRef(THIS)
{
	++m_ulRef;
	return m_pUnk->AddRef();
}

STDMETHODIMP_(ULONG) HackerIndexBuffer9::Release(THIS)
{
	LogDebug("HackerIndexBuffer9::Release handle=%x, counter=%d, this=%x\n", m_pUnk, m_ulRef, this);
	
    ULONG ulRef = m_pUnk ? m_pUnk->Release() : 0;
	LogDebug("  internal counter = %d\n", ulRef);
	
	--m_ulRef;

    if (ulRef == 0)
    {
		if (!gLogDebug) LogInfo("HackerIndexBuffer9::Release handle=%x, counter=%d, internal counter = %d\n", m_pUnk, m_ulRef, ulRef);
		LogInfo("  deleting self\n");
		
        if (m_pUnk) m_List.DeleteMember(m_pUnk); 
		m_pUnk = 0;
        delete this;
        return 0L;
    }
    return ulRef;
}

STDMETHODIMP HackerIndexBuffer9::GetDevice(THIS_ HackerDevice9** ppDevice)
{
	LogDebug("HackerIndexBuffer9::GetDevice called\n");
	
	CheckIndexBuffer9(this);
	IDirect3DDevice9 *origDevice;
	HRESULT hr = GetD3DIndexBuffer9()->GetDevice(&origDevice);
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

STDMETHODIMP HackerIndexBuffer9::SetPrivateData(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags)
{
	LogDebug("HackerIndexBuffer9::SetPrivateData called\n");
	
	CheckIndexBuffer9(this);
	return GetD3DIndexBuffer9()->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

STDMETHODIMP HackerIndexBuffer9::GetPrivateData(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData)
{
	LogDebug("HackerIndexBuffer9::GetPrivateData called\n");
	
	CheckIndexBuffer9(this);
	return GetD3DIndexBuffer9()->GetPrivateData(refguid, pData, pSizeOfData);
}

STDMETHODIMP HackerIndexBuffer9::FreePrivateData(THIS_ REFGUID refguid)
{
	LogDebug("HackerIndexBuffer9::GetPrivateData called\n");
	
	CheckIndexBuffer9(this);
	return GetD3DIndexBuffer9()->FreePrivateData(refguid);
}

STDMETHODIMP_(DWORD) HackerIndexBuffer9::SetPriority(THIS_ DWORD PriorityNew)
{
	LogDebug("HackerIndexBuffer9::SetPriority called\n");
	
	CheckIndexBuffer9(this);
	return GetD3DIndexBuffer9()->SetPriority(PriorityNew);
}

STDMETHODIMP_(DWORD) HackerIndexBuffer9::GetPriority(THIS)
{
	LogDebug("HackerIndexBuffer9::GetPriority called\n");
	
	CheckIndexBuffer9(this);
	return GetD3DIndexBuffer9()->GetPriority();
}

STDMETHODIMP_(void) HackerIndexBuffer9::PreLoad(THIS)
{
	LogDebug("HackerIndexBuffer9::GetPriority called\n");
	
	CheckIndexBuffer9(this);
	return GetD3DIndexBuffer9()->PreLoad();
}

STDMETHODIMP_(D3DRESOURCETYPE) HackerIndexBuffer9::GetType(THIS)
{
	LogDebug("HackerIndexBuffer9::GetType called\n");
	
	CheckIndexBuffer9(this);
	D3DRESOURCETYPE hr = GetD3DIndexBuffer9()->GetType();
	LogDebug("  returns ResourceType=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerIndexBuffer9::Lock(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags)
{
	LogDebug("HackerIndexBuffer9::Lock called with OffsetToLock=%d, SizeToLock=%d, Flags=%x\n", OffsetToLock, SizeToLock, Flags);
	
	if (!GetD3DIndexBuffer9())
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

	HRESULT hr = GetD3DIndexBuffer9()->Lock(OffsetToLock, SizeToLock, ppbData, Flags);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerIndexBuffer9::Unlock(THIS)
{
	LogDebug("HackerIndexBuffer9::Unlock called\n");

	if (!GetD3DIndexBuffer9())
	{
		if (!gLogDebug) LogInfo("HackerIndexBuffer9::Unlock called\n");
		LogInfo("  postponing call because vertex buffer was not created yet.\n");
		
		return S_OK;
	}

	CheckIndexBuffer9(this);
	HRESULT hr = GetD3DIndexBuffer9()->Unlock();
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerIndexBuffer9::GetDesc(THIS_ D3DINDEXBUFFER_DESC *pDesc)
{
	LogDebug("HackerIndexBuffer9::GetDesc called\n");
	
	CheckIndexBuffer9(this);
	return GetD3DIndexBuffer9()->GetDesc(pDesc);
}
