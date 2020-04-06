HackerSwapChain9::HackerSwapChain9( LPDIRECT3DSWAPCHAIN9EX pSwapChain, HackerDevice9* pDevice )
    : IDirect3DUnknown((IUnknown*) pSwapChain),
	pendingGetSwapChain(false)
{
//    m_pDevice = pDevice;
}

HackerSwapChain9* HackerSwapChain9::GetSwapChain(LPDIRECT3DSWAPCHAIN9EX pSwapChain, HackerDevice9* pDevice )
{
    HackerSwapChain9* p = (HackerSwapChain9*) m_List.GetDataPtr(pSwapChain);
    if (!p)
    {
        p = new HackerSwapChain9(pSwapChain, pDevice);
        if (pSwapChain) m_List.AddMember(pSwapChain, p);
    }
    return p;
}

STDMETHODIMP_(ULONG) HackerSwapChain9::AddRef(THIS)
{
	++m_ulRef;
	return m_pUnk->AddRef();
}

STDMETHODIMP_(ULONG) HackerSwapChain9::Release(THIS)
{
	LogDebug("HackerSwapChain9::Release handle=%x, counter=%d, this=%x\n", m_pUnk, m_ulRef, this);
	
    ULONG ulRef = m_pUnk ? m_pUnk->Release() : 0;
	LogDebug("  internal counter = %d\n", ulRef);
	
	--m_ulRef;

    if (ulRef == 0)
    {
		if (!gLogDebug) LogInfo("HackerSwapChain9::Release handle=%x, counter=%d, internal counter = %d\n", m_pUnk, m_ulRef, ulRef);
		LogInfo("  deleting self\n");
		
        if (m_pUnk) m_List.DeleteMember(m_pUnk); 
		m_pUnk = 0;
        delete this;
        return 0L;
    }
    return ulRef;
}

static void CheckSwapChain(HackerSwapChain9 *me)
{
	if (!me->pendingGetSwapChain)
		return;
	me->pendingGetSwapChain = false;
	CheckDevice(me->pendingDevice);

	LogInfo("  calling postponed GetSwapChain.\n");
	LPDIRECT3DSWAPCHAIN9 baseSwapChain = NULL;
	HRESULT hr = me->pendingDevice->GetD3D9Device()->GetSwapChain(me->_SwapChain, &baseSwapChain);
	if (FAILED(hr))
	{
		LogInfo("    failed getting swap chain with result = %x\n", hr);
		
		return;
	}
	const IID IID_HackerSwapChain9Ex = { 0x91886caf, 0x1c3d, 0x4d2e, { 0xa0, 0xab, 0x3e, 0x4c, 0x7d, 0x8d, 0x33, 0x3 }};
	hr = baseSwapChain->QueryInterface(IID_HackerSwapChain9Ex, (void **)&me->m_pUnk);
	baseSwapChain->Release();
	if (FAILED(hr))
	{
		LogInfo("    failed casting swap chain to HackerSwapChain9 with result = %x\n", hr);
		
		return;
	}	
}

STDMETHODIMP HackerSwapChain9::Present(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags)
{
	LogDebug("HackerSwapChain9::Present called\n");
	
	CheckSwapChain(this);
	HRESULT hr = GetSwapChain9()->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerSwapChain9::GetFrontBufferData(THIS_ HackerSurface9 *pDestSurface)
{
	LogDebug("HackerSwapChain9::GetFrontBufferData called\n");
	
	CheckSwapChain(this);
	HRESULT hr = GetSwapChain9()->GetFrontBufferData(replaceSurface9(pDestSurface));
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerSwapChain9::GetBackBuffer(THIS_ UINT iBackBuffer,D3DBACKBUFFER_TYPE Type, HackerSurface9 **ppBackBuffer)
{
	LogDebug("HackerSwapChain9::GetBackBuffer called\n");
	
	CheckSwapChain(this);
	LPDIRECT3DSURFACE9 baseSurface = 0;
    HRESULT hr = GetSwapChain9()->GetBackBuffer(iBackBuffer, Type, &baseSurface);
    if (FAILED(hr) || !baseSurface)
    {
		LogInfo("  failed with hr = %x\n", hr);
		
		if (ppBackBuffer) *ppBackBuffer = 0;
        return hr;
    }
	if (ppBackBuffer && baseSurface) *ppBackBuffer = HackerSurface9::GetDirect3DSurface9(baseSurface);
	if (ppBackBuffer) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseSurface, *ppBackBuffer);
	
    return hr;
}

STDMETHODIMP HackerSwapChain9::GetRasterStatus(THIS_ D3DRASTER_STATUS* pRasterStatus)
{
	LogDebug("HackerSwapChain9::GetRasterStatus called\n");
	
	CheckSwapChain(this);
	return GetSwapChain9()->GetRasterStatus(pRasterStatus);
}

STDMETHODIMP HackerSwapChain9::GetDisplayMode(THIS_ D3DDISPLAYMODE* pMode)
{
	LogDebug("HackerSwapChain9::GetDisplayMode called\n");
	
	CheckSwapChain(this);
	return GetSwapChain9()->GetDisplayMode(pMode);
}

STDMETHODIMP HackerSwapChain9::GetDevice(THIS_ HackerDevice9** ppDevice)
{
	LogDebug("HackerSwapChain9::GetDevice called\n");
	
	CheckSwapChain(this);
	IDirect3DDevice9 *origDevice;
	HRESULT hr = GetSwapChain9()->GetDevice(&origDevice);
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

STDMETHODIMP HackerSwapChain9::GetPresentParameters(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	LogDebug("HackerSwapChain9::GetPresentParameters called.\n");
	
	CheckSwapChain(this);
	return GetSwapChain9()->GetPresentParameters(pPresentationParameters);
}

STDMETHODIMP HackerSwapChain9::GetDisplayModeEx(THIS_ D3DDISPLAYMODEEX* pMode,D3DDISPLAYROTATION* pRotation)
{
	LogDebug("HackerSwapChain9::GetDisplayModeEx called.\n");
	
	CheckSwapChain(this);
	return GetSwapChain9()->GetDisplayModeEx(pMode, pRotation);
}
