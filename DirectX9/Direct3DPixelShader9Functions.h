
HackerPixelShader9::HackerPixelShader9(LPDIRECT3DPIXELSHADER9 pVS)
    : IDirect3DUnknown((IUnknown*) pVS),
	magic(0x7da43feb)
{
}

HackerPixelShader9* HackerPixelShader9::GetDirect3DPixelShader9(LPDIRECT3DPIXELSHADER9 pVS)
{
    HackerPixelShader9* p = (HackerPixelShader9*) m_List.GetDataPtr(pVS);
    if (!p)
    {
        p = new HackerPixelShader9(pVS);
        if (pVS) m_List.AddMember(pVS, p);
    }
    return p;
}

STDMETHODIMP_(ULONG) HackerPixelShader9::AddRef(THIS)
{
	++m_ulRef;
	return m_pUnk->AddRef();
}

STDMETHODIMP_(ULONG) HackerPixelShader9::Release(THIS)
{
	LogDebug("HackerPixelShader9::Release handle=%x, counter=%d, this=%x\n", m_pUnk, m_ulRef, this);
	
    ULONG ulRef = m_pUnk ? m_pUnk->Release() : 0;
	LogDebug("  internal counter = %d\n", ulRef);
	
	--m_ulRef;

    if (ulRef == 0)
    {
		if (!gLogDebug) LogInfo("HackerPixelShader9::Release handle=%x, counter=%d, internal counter = %d\n", m_pUnk, m_ulRef, ulRef);
		LogInfo("  deleting self\n");
		
        if (m_pUnk) m_List.DeleteMember(m_pUnk); 
		m_pUnk = 0;
        delete this;
        return 0L;
    }
    return ulRef;
}

STDMETHODIMP HackerPixelShader9::GetDevice(THIS_ HackerDevice9** ppDevice)
{
	LogDebug("HackerPixelShader9::GetDevice called\n");
	
	IDirect3DDevice9 *origDevice;
	HRESULT hr = GetD3DPixelShader9()->GetDevice(&origDevice);
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

STDMETHODIMP HackerPixelShader9::GetFunction(THIS_ void *data,UINT* pSizeOfData)
{
	LogDebug("HackerVertexShader9::GetFunction called\n");
	
	HRESULT hr = GetD3DPixelShader9()->GetFunction(data, pSizeOfData);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}
