
HackerVertexDeclaration9::HackerVertexDeclaration9(LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration)
    : IDirect3DUnknown((IUnknown*) pVertexDeclaration),
	pendingCreateVertexDeclaration(false),
	magic(0x7da43feb)
{
}

HackerVertexDeclaration9* HackerVertexDeclaration9::GetDirect3DVertexDeclaration9(LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration)
{
    HackerVertexDeclaration9* p = (HackerVertexDeclaration9*) m_List.GetDataPtr(pVertexDeclaration);
    if (p == NULL)
    {
        p = new HackerVertexDeclaration9(pVertexDeclaration);
		if (pVertexDeclaration) m_List.AddMember(pVertexDeclaration, p);
    }
    return p;
}

STDMETHODIMP_(ULONG) HackerVertexDeclaration9::AddRef(THIS)
{
	++m_ulRef;
	return m_pUnk->AddRef();
}

STDMETHODIMP_(ULONG) HackerVertexDeclaration9::Release(THIS)
{
	LogDebug("HackerVertexDeclaration9::Release handle=%x, counter=%d, this=%x\n", m_pUnk, m_ulRef, this);

	ULONG ulRef = m_pUnk ? m_pUnk->Release() : 0;
	LogDebug("  internal counter = %d\n", ulRef);

	--m_ulRef;

    if (ulRef == 0)
    {
		if (!gLogDebug) LogInfo("HackerVertexDeclaration9::Release handle=%x, counter=%d, internal counter = %d\n", m_pUnk, m_ulRef, ulRef);
		LogInfo("  deleting self\n");
		
        if (m_pUnk) m_List.DeleteMember(m_pUnk); 
		m_pUnk = 0;
        delete this;
        return 0L;
    }
    return ulRef;
}

STDMETHODIMP HackerVertexDeclaration9::GetDevice(THIS_ HackerDevice9** ppDevice)
{
	LogDebug("HackerVertexDeclaration9::GetDevice called\n");

	CheckVertexDeclaration9(this);
	IDirect3DDevice9 *origDevice;
	HRESULT hr = GetD3DVertexDeclaration9()->GetDevice(&origDevice);
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

STDMETHODIMP HackerVertexDeclaration9::GetDeclaration(THIS_ D3DVERTEXELEMENT9* pElement,UINT* pNumElements)
{
	LogDebug("HackerVertexDeclaration9::GetDeclaration called\n");

	CheckVertexDeclaration9(this);
	return GetD3DVertexDeclaration9()->GetDeclaration(pElement, pNumElements);
}
