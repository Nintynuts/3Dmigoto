
HackerQuery9::HackerQuery9(LPDIRECT3DQUERY9 pQuery)
    : IDirect3DUnknown((IUnknown*) pQuery),
	magic(0x7da43feb)
{
}

HackerQuery9* HackerQuery9::GetDirect3DQuery9(LPDIRECT3DQUERY9 pIndexBuffer)
{
    HackerQuery9* p = (HackerQuery9*) m_List.GetDataPtr(pIndexBuffer);
    if (!p)
    {
        p = new HackerQuery9(pIndexBuffer);
        if (pIndexBuffer) m_List.AddMember(pIndexBuffer, p);
    }
    return p;
}

STDMETHODIMP_(ULONG) HackerQuery9::AddRef(THIS)
{
	++m_ulRef;
	return m_pUnk->AddRef();
}

STDMETHODIMP_(ULONG) HackerQuery9::Release(THIS)
{
	LogDebug("HackerQuery9::Release handle=%x, counter=%d, this=%x\n", m_pUnk, m_ulRef, this);
	
    ULONG ulRef = m_pUnk ? m_pUnk->Release() : 0;
	LogDebug("  internal counter = %d\n", ulRef);
	
	--m_ulRef;

    if (ulRef == 0)
    {
		if (!gLogDebug) LogInfo("HackerQuery9::Release handle=%x, counter=%d, internal counter = %d\n", m_pUnk, m_ulRef, ulRef);
		LogInfo("  deleting self\n");
		
        if (m_pUnk) m_List.DeleteMember(m_pUnk); 
		m_pUnk = 0;
        delete this;
        return 0L;
    }
    return ulRef;
}

STDMETHODIMP HackerQuery9::GetDevice(THIS_ HackerDevice9** ppDevice)
{
	LogDebug("HackerQuery9::GetDevice called\n");
	
	IDirect3DDevice9 *origDevice;
	HRESULT hr = GetD3DQuery9()->GetDevice(&origDevice);
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

STDMETHODIMP_(D3DQUERYTYPE) HackerQuery9::GetType(THIS)
{
	LogDebug("HackerQuery9::GetType called\n");
	
	D3DQUERYTYPE hr = GetD3DQuery9()->GetType();
	LogDebug("  returns QueryType=%x\n", hr);
	
	return hr;
}

STDMETHODIMP_(DWORD) HackerQuery9::GetDataSize(THIS)
{
	LogDebug("HackerQuery9::GetDataSize called\n");
	
	DWORD hr = GetD3DQuery9()->GetDataSize();
	LogDebug("  returns size=%d\n", hr);
	
	return hr;
}

STDMETHODIMP HackerQuery9::Issue(THIS_ DWORD dwIssueFlags)
{
	LogDebug("HackerQuery9::Issue called with IssueFlags=%x, this=%x\n", dwIssueFlags, this);
	
	HRESULT hr = GetD3DQuery9()->Issue(dwIssueFlags);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerQuery9::GetData(THIS_ void* pData,DWORD dwSize,DWORD dwGetDataFlags)
{
	LogDebug("HackerQuery9::GetData called with Data=%x, Size=%d, GetDataFlags=%x\n", pData, dwSize, dwGetDataFlags);
	
	HRESULT hr = GetD3DQuery9()->GetData(pData, dwSize, dwGetDataFlags);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}
