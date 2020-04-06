
HackerDevice9::HackerDevice9(LPDIRECT3DDEVICE9EX pDevice)
    : IDirect3DUnknown((IUnknown*) pDevice),
	pendingCreateDepthStencilSurface(0),
	pendingSetDepthStencilSurface(0)
{
}

HackerDevice9* HackerDevice9::GetDirect3DDevice(LPDIRECT3DDEVICE9EX pDevice)
{
    HackerDevice9* p = (HackerDevice9*) m_List.GetDataPtr(pDevice);
    if (!p)
    {
        p = new HackerDevice9(pDevice);
		if (pDevice) m_List.AddMember(pDevice, p);
    }
    return p;
}

STDMETHODIMP_(ULONG) HackerDevice9::AddRef(THIS)
{
	++m_ulRef;
	return m_pUnk->AddRef();
}

STDMETHODIMP_(ULONG) HackerDevice9::Release(THIS)
{
	LogDebug("HackerDevice9::Release handle=%x, counter=%d, this=%x\n", m_pUnk, m_ulRef, this);
	
    ULONG ulRef = m_pUnk ? m_pUnk->Release() : 0;
	LogDebug("  internal counter = %d\n", ulRef);
	
	--m_ulRef;

    if (ulRef == 0)
    {
		if (!gLogDebug) LogInfo("HackerDevice9::Release handle=%x, counter=%d, internal counter = %d\n", m_pUnk, m_ulRef, ulRef);
		LogInfo("  deleting self\n");
		
        if (m_pUnk) m_List.DeleteMember(m_pUnk); 
		m_pUnk = 0;
        delete this;
        return 0L;
    }
    return ulRef;
}

STDMETHODIMP HackerDevice9::TestCooperativeLevel(THIS)
{
	LogInfo("HackerDevice9::TestCooperativeLevel called\n");
	
	CheckDevice(this);
	return GetD3D9Device()->TestCooperativeLevel();
}

STDMETHODIMP_(UINT) HackerDevice9::GetAvailableTextureMem(THIS)
{
	LogInfo("HackerDevice9::GetAvailableTextureMem called\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetAvailableTextureMem();
}

STDMETHODIMP HackerDevice9::EvictManagedResources(THIS)
{
	LogInfo("HackerDevice9::EvictManagedResources called\n");
	
	CheckDevice(this);
	return GetD3D9Device()->EvictManagedResources();
}

STDMETHODIMP HackerDevice9::GetDirect3D(THIS_ Hacker9** ppD3D9)
{
	LogInfo("HackerDevice9::GetDirect3D called\n");
	
	CheckDevice(this);
    LPDIRECT3D9 baseDirect3D = NULL;
    HRESULT hr = GetD3D9Device()->GetDirect3D(&baseDirect3D);
    if (FAILED(hr))
    {
		LogInfo("  call failed with hr=%x\n", hr);
		
        if (ppD3D9) *ppD3D9 = NULL;
        return hr;
    }
	// Cast to LPDIRECT3D9EX
	LPDIRECT3D9EX baseDirect3DEx = NULL;
	const IID IID_Hacker9Ex = { 0x02177241, 0x69FC, 0x400C, { 0x8F, 0xF1, 0x93, 0xA4, 0x4D, 0xF6, 0x86, 0x1D }};
    hr = baseDirect3D->QueryInterface(IID_Hacker9Ex, (void **)&baseDirect3DEx);
	baseDirect3D->Release();
    if (FAILED(hr))
    {
		LogInfo("  cast to Hacker9Ex failed with hr=%x\n", hr);
		
        if (ppD3D9) *ppD3D9 = NULL;
        return hr;
    }

	Hacker9* pD3D = Hacker9::GetDirect3D(baseDirect3DEx);
    if (ppD3D9) *ppD3D9 = pD3D;
	LogInfo("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseDirect3DEx, pD3D);
	
    return hr;
}

STDMETHODIMP HackerDevice9::GetDeviceCaps(THIS_ D3DCAPS9* pCaps)
{
	LogInfo("HackerDevice9::GetDeviceCaps called\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetDeviceCaps(pCaps);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetDisplayMode(THIS_ UINT iSwapChain,D3DDISPLAYMODE* pMode)
{
	LogInfo("HackerDevice9::GetDisplayMode called\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetDisplayMode(iSwapChain, pMode);
	if (hr == S_OK && pMode)
	{
		if (SCREEN_REFRESH != -1 && pMode->RefreshRate != SCREEN_REFRESH)
		{
			LogInfo("  overriding refresh rate %d with %d\n", pMode->RefreshRate, SCREEN_REFRESH);
			
			pMode->RefreshRate = SCREEN_REFRESH;
		}
	}
	if (!pMode) LogInfo("  returns result=%x\n", hr);
	if (pMode) LogInfo("  returns result=%x, Width=%d, Height=%d, RefreshRate=%d, Format=%d\n", hr,
		pMode->Width, pMode->Height, pMode->RefreshRate, pMode->Format);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetCreationParameters(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	LogInfo("HackerDevice9::GetCreationParameters called\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetCreationParameters(pParameters);
	if (!pParameters) LogInfo("  returns result=%x\n", hr);
	if (pParameters) LogInfo("  returns result=%x, AdapterOrdinal=%d, DeviceType=%d, FocusWindow=%x, BehaviorFlags=%x\n", hr,
		pParameters->AdapterOrdinal, pParameters->DeviceType, pParameters->hFocusWindow, pParameters->BehaviorFlags);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetCursorProperties(THIS_ UINT XHotSpot,UINT YHotSpot, HackerSurface9 *pCursorBitmap)
{
	LogInfo("HackerDevice9::SetCursorProperties called\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetCursorProperties(XHotSpot, YHotSpot, replaceSurface9(pCursorBitmap));
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP_(void) HackerDevice9::SetCursorPosition(THIS_ int X,int Y,DWORD Flags)
{
	LogInfo("HackerDevice9::SetCursorPosition called\n");
	
	CheckDevice(this);
	GetD3D9Device()->SetCursorPosition(X, Y, Flags);
}

STDMETHODIMP_(BOOL) HackerDevice9::ShowCursor(THIS_ BOOL bShow)
{
	LogInfo("HackerDevice9::ShowCursor called\n");
	
	CheckDevice(this);
	BOOL hr = GetD3D9Device()->ShowCursor(bShow);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::CreateAdditionalSwapChain(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,HackerSwapChain9** pSwapChain)
{
	LogInfo("HackerDevice9::CreateAdditionalSwapChain called\n");
	
	CheckDevice(this);

	IDirect3DSwapChain9 *baseSwapChain;
	HRESULT hr = GetD3D9Device()->CreateAdditionalSwapChain(pPresentationParameters, &baseSwapChain);
    if (FAILED(hr) || baseSwapChain == NULL)
    {
		LogInfo("  failed with hr=%x\n", hr);
		
        return hr;
    }
	LPDIRECT3DSWAPCHAIN9EX baseSwapChainEx = NULL;
	const IID IID_HackerSwapChain9Ex = { 0x91886caf, 0x1c3d, 0x4d2e, { 0xa0, 0xab, 0x3e, 0x4c, 0x7d, 0x8d, 0x33, 0x3 }};
	hr = baseSwapChain->QueryInterface(IID_HackerSwapChain9Ex, (void **)&baseSwapChainEx);
	baseSwapChain->Release();
	if (FAILED(hr) || baseSwapChainEx == NULL)
	{
		LogInfo("  cast to HackerSwapChain9Ex failed with hr=%x\n", hr);
		
		return hr;
	}
    
    HackerSwapChain9* NewSwapChain = HackerSwapChain9::GetSwapChain(baseSwapChainEx, this);
    if (NewSwapChain == NULL)
    {
		LogInfo("  error creating wrapper\n", hr);
		
        baseSwapChainEx->Release();
        return E_OUTOFMEMORY;
    }
	if (pSwapChain) *pSwapChain = NewSwapChain;
	LogInfo("  returns result=%x, handle=%x\n", hr, NewSwapChain);
	
    return hr;
}

STDMETHODIMP HackerDevice9::GetSwapChain(THIS_ UINT iSwapChain,HackerSwapChain9** pSwapChain)
{
	LogInfo("HackerDevice9::GetSwapChain called with SwapChain=%d\n", iSwapChain);
	

	if (!GetD3D9Device())
	{
		LogInfo("  postponing call because device was not created yet.\n");
		
		HackerSwapChain9 *wrapper = HackerSwapChain9::GetSwapChain((LPDIRECT3DSWAPCHAIN9EX) 0, this);
		wrapper->_SwapChain = iSwapChain;
		wrapper->pendingGetSwapChain = true;
		wrapper->pendingDevice = this;
		*pSwapChain = wrapper;
		LogInfo("  returns handle=%x\n", wrapper);
		
		return S_OK;
	}

    *pSwapChain = NULL;
	LPDIRECT3DSWAPCHAIN9 baseSwapChain = NULL;
    HRESULT hr = GetD3D9Device()->GetSwapChain(iSwapChain, &baseSwapChain);
    if (FAILED(hr) || baseSwapChain == NULL)
    {
		LogInfo("  failed with hr=%x\n", hr);
		
        return hr;
    }
	LPDIRECT3DSWAPCHAIN9EX baseSwapChainEx = NULL;
	const IID IID_HackerSwapChain9Ex = { 0x91886caf, 0x1c3d, 0x4d2e, { 0xa0, 0xab, 0x3e, 0x4c, 0x7d, 0x8d, 0x33, 0x3 }};
	hr = baseSwapChain->QueryInterface(IID_HackerSwapChain9Ex, (void **)&baseSwapChainEx);
	baseSwapChain->Release();
	if (FAILED(hr) || baseSwapChainEx == NULL)
	{
		LogInfo("  cast to HackerSwapChain9Ex failed with hr=%x\n", hr);
		
		return hr;
	}
    
    HackerSwapChain9* newSwapChain = HackerSwapChain9::GetSwapChain(baseSwapChainEx, this);
    if (newSwapChain == NULL)
    {
		LogInfo("  error creating wrapper\n", hr);
		
        baseSwapChainEx->Release();
        return E_OUTOFMEMORY;
    }
	if (pSwapChain) *pSwapChain = newSwapChain;
	LogInfo("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseSwapChainEx, newSwapChain);
	
    return hr;
}

STDMETHODIMP_(UINT) HackerDevice9::GetNumberOfSwapChains(THIS)
{
	LogInfo("GetNumberOfSwapChains\n");
	
	CheckDevice(this);
	UINT hr = GetD3D9Device()->GetNumberOfSwapChains();
	LogInfo("  returns NumberOfSwapChains=%d\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::Reset(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	LogInfo("HackerDevice9::Reset called on handle=%x\n", GetD3D9Device());
	
	CheckDevice(this);
	LogInfo("  BackBufferWidth %d\n", pPresentationParameters->BackBufferWidth);
	LogInfo("  BackBufferHeight %d\n", pPresentationParameters->BackBufferHeight);
	LogInfo("  BackBufferFormat %x\n", pPresentationParameters->BackBufferFormat);
	LogInfo("  BackBufferCount %d\n", pPresentationParameters->BackBufferCount);
	LogInfo("  SwapEffect %x\n", pPresentationParameters->SwapEffect);
	LogInfo("  Flags %x\n", pPresentationParameters->Flags);
	LogInfo("  FullScreen_RefreshRateInHz %d\n", pPresentationParameters->FullScreen_RefreshRateInHz);
	LogInfo("  PresentationInterval %d\n", pPresentationParameters->PresentationInterval);
	LogInfo("  Windowed %d\n", pPresentationParameters->Windowed);
	LogInfo("  EnableAutoDepthStencil %d\n", pPresentationParameters->EnableAutoDepthStencil);
	LogInfo("  AutoDepthStencilFormat %d\n", pPresentationParameters->AutoDepthStencilFormat);

	if (SCREEN_REFRESH >= 0) 
	{
		LogInfo("    overriding refresh rate = %d\n", SCREEN_REFRESH);
		pPresentationParameters->FullScreen_RefreshRateInHz = SCREEN_REFRESH;
	}
	if (SCREEN_WIDTH >= 0) 
	{
		LogInfo("    overriding width = %d\n", SCREEN_WIDTH);
		pPresentationParameters->BackBufferWidth = SCREEN_WIDTH;
	}
	if (SCREEN_HEIGHT >= 0) 
	{
		LogInfo("    overriding height = %d\n", SCREEN_HEIGHT);
		pPresentationParameters->BackBufferHeight = SCREEN_HEIGHT;
	}
	if (SCREEN_FULLSCREEN >= 0 && SCREEN_FULLSCREEN < 2)
	{
		LogInfo("    overriding full screen = %d\n", SCREEN_FULLSCREEN);
		pPresentationParameters->Windowed = !SCREEN_FULLSCREEN;
	}

//	pPresentationParameters->SwapEffect = D3DSWAPEFFECT_DISCARD;
//	pPresentationParameters->PresentationInterval = 0; // D3DPRESENT_INTERVAL_DEFAULT
//	pPresentationParameters->EnableAutoDepthStencil = TRUE;
//	pPresentationParameters->AutoDepthStencilFormat = D3DFMT_D24X8; // 77
//	pPresentationParameters->BackBufferFormat = D3DFMT_A8R8G8B8; // 21

	HRESULT hr = GetD3D9Device()->Reset(pPresentationParameters);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

UINT FrameIndex = 0;
STDMETHODIMP HackerDevice9::Present(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	LogDebug("HackerDevice9::Present called.\n");
	
	CheckDevice(this);

	if (SCREEN_FULLSCREEN == 2)
	{
		if (!gLogDebug) LogInfo("HackerDevice9::Present called.\n");
		LogInfo("  initiating reset to switch to full screen.\n");
		
		SCREEN_FULLSCREEN = 1;
		SCREEN_WIDTH = SCREEN_WIDTH_DELAY;
		SCREEN_HEIGHT = SCREEN_HEIGHT_DELAY;
		SCREEN_REFRESH = SCREEN_REFRESH_DELAY;
		return D3DERR_DEVICELOST;
	}

	HRESULT hr = GetD3D9Device()->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	if (hr == D3DERR_DEVICELOST)
	{
		LogInfo("  returns D3DERR_DEVICELOST\n");
	}
	else
	{
		LogDebug("  returns result=%x\n", hr);
	}
	return hr;
}

STDMETHODIMP HackerDevice9::GetBackBuffer(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type, HackerSurface9 **ppBackBuffer)
{
	LogInfo("HackerDevice9::GetBackBuffer called\n");
	
	CheckDevice(this);

	LPDIRECT3DSURFACE9 baseSurface = 0;
    HRESULT hr = GetD3D9Device()->GetBackBuffer(iSwapChain, iBackBuffer, Type, &baseSurface);
	if (FAILED(hr) || !baseSurface)
    {
		LogInfo("  failed with hr=%x\n", hr);
		
		if (ppBackBuffer) *ppBackBuffer = 0;
        return hr;
    }
	if (ppBackBuffer && baseSurface) *ppBackBuffer = HackerSurface9::GetDirect3DSurface9(baseSurface);
	if (ppBackBuffer) LogInfo("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseSurface, *ppBackBuffer);
	
    return hr;
}

STDMETHODIMP HackerDevice9::GetRasterStatus(THIS_ UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{
	LogDebug("HackerDevice9::GetRasterStatus called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetRasterStatus(iSwapChain, pRasterStatus);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetDialogBoxMode(THIS_ BOOL bEnableDialogs)
{
	LogDebug("HackerDevice9::SetDialogBoxMode called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetDialogBoxMode(bEnableDialogs);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP_(void) HackerDevice9::SetGammaRamp(THIS_ UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
	LogDebug("HackerDevice9::SetGammaRamp called.\n");
	
	CheckDevice(this);
	GetD3D9Device()->SetGammaRamp(iSwapChain, Flags, pRamp);
}

STDMETHODIMP_(void) HackerDevice9::GetGammaRamp(THIS_ UINT iSwapChain,D3DGAMMARAMP* pRamp)
{
	LogDebug("HackerDevice9::GetGammaRamp called.\n");
	
	CheckDevice(this);
	GetD3D9Device()->GetGammaRamp(iSwapChain, pRamp);
}

STDMETHODIMP HackerDevice9::CreateTexture(THIS_ UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool, HackerTexture9 **ppTexture,HANDLE* pSharedHandle)
{
	LogDebug("HackerDevice9::CreateTexture called with Width=%d, Height=%d, Levels=%d, Usage=%x, Format=%d, Pool=%d, SharedHandle=%x\n", 
		Width, Height, Levels, Usage, Format, Pool, pSharedHandle);

	if (!GetD3D9Device())
	{
		LogInfo("  postponing call because device was not created yet.\n");
		
		HackerTexture9 *wrapper = HackerTexture9::GetDirect3DVertexDeclaration9((LPDIRECT3DTEXTURE9) 0);
		wrapper->_Width = Width;
		wrapper->_Height = Height;
		wrapper->_Levels = Levels;
		wrapper->_Usage = Usage;
		wrapper->_Format = Format;
		wrapper->_Pool = Pool;
		wrapper->_Device = this;
		wrapper->pendingCreateTexture = true;
		*ppTexture = wrapper;
		LogInfo("  returns handle=%x\n", wrapper);
		
		return S_OK;
	}

	LPDIRECT3DTEXTURE9 baseTexture = 0;
	if (Pool == D3DPOOL_MANAGED)
	{
		LogDebug("  Pool changed from MANAGED to DEFAULT because of DirectX9Ex migration.\n");
		
		Pool = D3DPOOL_DEFAULT;
	}
    HRESULT hr = GetD3D9Device()->CreateTexture(Width, Height, Levels, Usage, Format, Pool, &baseTexture, pSharedHandle);
	if (ppTexture && baseTexture) *ppTexture = HackerTexture9::GetDirect3DVertexDeclaration9(baseTexture);	
	if (ppTexture) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseTexture, *ppTexture);
	
	return hr;
}

STDMETHODIMP HackerDevice9::CreateVolumeTexture(THIS_ UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
	LogInfo("HackerDevice9::CreateVolumeTexture Width=%d Height=%d Format=%d\n", Width, Height, Format);
	
	CheckDevice(this);
	if (Pool == D3DPOOL_MANAGED)
	{
		LogDebug("  Pool changed from MANAGED to DEFAULT because of DirectX9Ex migration.\n");
		
		Pool = D3DPOOL_DEFAULT;
	}
	HRESULT hr = GetD3D9Device()->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::CreateCubeTexture(THIS_ UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
	LogInfo("HackerDevice9::CreateCubeTexture EdgeLength=%d Format=%d\n", EdgeLength, Format);
	
	CheckDevice(this);
	if (Pool == D3DPOOL_MANAGED)
	{
		LogDebug("  Pool changed from MANAGED to DEFAULT because of DirectX9Ex migration.\n");
		
		Pool = D3DPOOL_DEFAULT;
	}
	HRESULT hr = GetD3D9Device()->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::CreateVertexBuffer(THIS_ UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool, HackerVertexBuffer9 **ppVertexBuffer,HANDLE* pSharedHandle)
{
	LogDebug("HackerDevice9::CreateVertexBuffer called with Length=%d\n", Length);

	if (!GetD3D9Device())
	{
		LogInfo("  postponing call because device was not created yet.\n");
		
		HackerVertexBuffer9 *wrapper = HackerVertexBuffer9::GetDirect3DVertexBuffer9((LPDIRECT3DVERTEXBUFFER9) 0);
		wrapper->_Length = Length;
		wrapper->_Usage = Usage;
		wrapper->_FVF = FVF;
		wrapper->_Pool = Pool;
		wrapper->_Device = this;
		wrapper->pendingCreateVertexBuffer = true;
		if (ppVertexBuffer) *ppVertexBuffer = wrapper;
		LogInfo("  returns handle=%x\n", wrapper);
		
		return S_OK;
	}

	LPDIRECT3DVERTEXBUFFER9 baseVB = 0;
	if (Pool == D3DPOOL_MANAGED)
	{
		LogDebug("  Pool changed from MANAGED to DEFAULT because of DirectX9Ex migration.\n");
		
		Pool = D3DPOOL_DEFAULT;
	}
    HRESULT hr = GetD3D9Device()->CreateVertexBuffer(Length, Usage, FVF, Pool, &baseVB, pSharedHandle);
	if (ppVertexBuffer && baseVB) *ppVertexBuffer = HackerVertexBuffer9::GetDirect3DVertexBuffer9(baseVB);
	if (ppVertexBuffer) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseVB, *ppVertexBuffer);
	    
    return hr;
}

STDMETHODIMP HackerDevice9::CreateIndexBuffer(THIS_ UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,
	HackerIndexBuffer9 **ppIndexBuffer,HANDLE* pSharedHandle)
{
	LogDebug("HackerDevice9::CreateIndexBuffer called with Length=%d, Usage=%x, Format=%d, Pool=%d, SharedHandle=%x, IndexBufferPtr=%x\n",
		Length, Usage, Format, Pool, pSharedHandle, ppIndexBuffer);

	if (!GetD3D9Device())
	{
		LogInfo("  postponing call because device was not created yet.\n");
		
		HackerIndexBuffer9 *wrapper = HackerIndexBuffer9::GetDirect3DIndexBuffer9((LPDIRECT3DINDEXBUFFER9) 0);
		wrapper->_Length = Length;
		wrapper->_Usage = Usage;
		wrapper->_Format = Format;
		wrapper->_Pool = Pool;
		wrapper->_Device = this;
		wrapper->pendingCreateIndexBuffer = true;
		if (ppIndexBuffer) *ppIndexBuffer = wrapper;
		LogInfo("  returns handle=%x\n", wrapper);
		
		return S_OK;
	}

	LPDIRECT3DINDEXBUFFER9 baseIB = 0;
	if (Pool == D3DPOOL_MANAGED)
	{
		LogDebug("  Pool changed from MANAGED to DEFAULT because of DirectX9Ex migration.\n");
		
		Pool = D3DPOOL_DEFAULT;
	}
    HRESULT hr = GetD3D9Device()->CreateIndexBuffer(Length, Usage, Format, Pool, &baseIB, pSharedHandle);
	if (ppIndexBuffer && baseIB) *ppIndexBuffer = HackerIndexBuffer9::GetDirect3DIndexBuffer9(baseIB);
	if (ppIndexBuffer) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseIB, *ppIndexBuffer);
	
    return hr;
}

STDMETHODIMP HackerDevice9::CreateRenderTarget(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable, HackerSurface9 **ppSurface,HANDLE* pSharedHandle)
{
	LogInfo("HackerDevice9::CreateRenderTarget called with Width=%d Height=%d Format=%d\n", Width, Height, Format);
	
	CheckDevice(this);

	LPDIRECT3DSURFACE9 baseSurface = NULL;
    HRESULT hr = GetD3D9Device()->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, &baseSurface, pSharedHandle);
    if (ppSurface && baseSurface) *ppSurface = HackerSurface9::GetDirect3DSurface9(baseSurface);
	if (ppSurface) LogInfo("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseSurface, *ppSurface);
	    
    return hr;
}

STDMETHODIMP HackerDevice9::CreateDepthStencilSurface(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,
																	  HackerSurface9** ppSurface,HANDLE* pSharedHandle)
{
	LogInfo("HackerDevice9::CreateDepthStencilSurface Width=%d Height=%d Format=%d\n", Width, Height, Format);

	if (!GetD3D9Device())
	{
		LogInfo("  postponing call because device was not created yet.\n");
		
		HackerSurface9 *wrapper = HackerSurface9::GetDirect3DSurface9((LPDIRECT3DSURFACE9) 0);
		wrapper->_Width = Width;
		wrapper->_Height = Height;
		wrapper->_Format = Format;
		wrapper->_MultiSample = MultiSample;
		wrapper->_MultisampleQuality = MultisampleQuality;
		wrapper->_Discard = Discard;
		wrapper->_Device = this;
		pendingCreateDepthStencilSurface = wrapper;
		*ppSurface = wrapper;
		LogInfo("  returns handle=%x\n", wrapper);
		
		return S_OK;
	}

	LPDIRECT3DSURFACE9 baseSurface = NULL;
    HRESULT hr = GetD3D9Device()->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, &baseSurface, pSharedHandle);
	if (ppSurface && baseSurface) *ppSurface = HackerSurface9::GetDirect3DSurface9(baseSurface);
	if (ppSurface) LogInfo("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseSurface, *ppSurface);
	    
    return hr;
}

STDMETHODIMP HackerDevice9::UpdateSurface(THIS_ HackerSurface9 *pSourceSurface,CONST RECT* pSourceRect,
														  HackerSurface9 *pDestinationSurface,CONST POINT* pDestPoint)
{
	LogDebug("HackerDevice9::UpdateSurface called with SourceSurface=%x, DestinationSurface=%x\n", pSourceSurface, pDestinationSurface);
	
	CheckDevice(this);
	LPDIRECT3DSURFACE9 baseSourceSurface = replaceSurface9(pSourceSurface);
	LPDIRECT3DSURFACE9 baseDestinationSurface = replaceSurface9(pDestinationSurface);
    HRESULT hr = GetD3D9Device()->UpdateSurface(baseSourceSurface, pSourceRect, baseDestinationSurface, pDestPoint);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::UpdateTexture(THIS_ LPDIRECT3DBASETEXTURE9 pSourceTexture,LPDIRECT3DBASETEXTURE9 pDestinationTexture)
{
    LogInfo("HackerDevice9::UpdateTexture called with SourceTexture=%x, DestinationTexture=%x\n", pSourceTexture, pDestinationTexture);

	HackerTexture9 *sourceTexture = (HackerTexture9*) HackerTexture9::m_List.GetDataPtr(pSourceTexture);
	HackerTexture9 *destinationTexture = (HackerTexture9*) HackerTexture9::m_List.GetDataPtr(pDestinationTexture);
	if (!sourceTexture && !destinationTexture)
	{
		sourceTexture = (HackerTexture9*)pSourceTexture;
		destinationTexture = (HackerTexture9*)pDestinationTexture;
		if (sourceTexture->pendingCreateTexture && destinationTexture->pendingCreateTexture && sourceTexture->pendingLockUnlock &&
			sourceTexture->_Width == destinationTexture->_Width && sourceTexture->_Height == destinationTexture->_Height)
		{
			LogInfo("  simulating texture update because both textures are not created yet.\n");
			
			if (!destinationTexture->pendingLockUnlock)
			{
				D3DLOCKED_RECT rect;
				destinationTexture->LockRect(0, &rect, 0, 0);
			}
			memcpy(destinationTexture->_Buffer, sourceTexture->_Buffer, sourceTexture->_Width*sourceTexture->_Height*4);
			return S_OK;
		}
	}

	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->UpdateTexture(replaceTexture9((HackerTexture9 *) pSourceTexture), replaceTexture9((HackerTexture9 *) pDestinationTexture));
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetRenderTargetData(THIS_ HackerSurface9 *pRenderTarget,
																HackerSurface9 *pDestSurface)
{
	LogInfo("HackerDevice9::GetRenderTargetData called with RenderTarget=%x, DestSurface=%x\n", pRenderTarget, pDestSurface);
	
	CheckDevice(this);

	LPDIRECT3DSURFACE9 baseRenderTarget = replaceSurface9(pRenderTarget);
	LPDIRECT3DSURFACE9 baseDestSurface = replaceSurface9(pDestSurface);
	HRESULT hr = GetD3D9Device()->GetRenderTargetData(baseRenderTarget, baseDestSurface);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetFrontBufferData(THIS_ UINT iSwapChain, HackerSurface9 *pDestSurface)
{
	LogInfo("HackerDevice9::GetFrontBufferData called with SwapChain=%d, DestSurface=%x\n", iSwapChain, pDestSurface);
	
	CheckDevice(this);

	LPDIRECT3DSURFACE9 baseDestSurface = replaceSurface9(pDestSurface);
	HRESULT hr = GetD3D9Device()->GetFrontBufferData(iSwapChain, baseDestSurface);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::StretchRect(THIS_ HackerSurface9 *pSourceSurface,CONST RECT* pSourceRect,
														HackerSurface9 *pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
	LogDebug("HackerDevice9::StretchRect called using SourceSurface=%x, DestSurface=%x\n", pSourceRect, pDestSurface);
	
	CheckDevice(this);
	LPDIRECT3DSURFACE9 baseSourceSurface = replaceSurface9(pSourceSurface);
	LPDIRECT3DSURFACE9 baseDestSurface = replaceSurface9(pDestSurface);
	HRESULT hr = GetD3D9Device()->StretchRect(baseSourceSurface, pSourceRect, baseDestSurface, pDestRect, Filter);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::ColorFill(THIS_ HackerSurface9 *pSurface,CONST RECT* pRect,D3DCOLOR color)
{
	LogDebug("HackerDevice9::ColorFill called with Surface=%x\n", pSurface);
	
	CheckDevice(this);
	LPDIRECT3DSURFACE9 baseSurface = replaceSurface9(pSurface);
	HRESULT hr = GetD3D9Device()->ColorFill(baseSurface, pRect, color);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::CreateOffscreenPlainSurface(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool, HackerSurface9 **ppSurface,HANDLE* pSharedHandle)
{
	LogInfo("HackerDevice9::CreateOffscreenPlainSurface called with Width=%d Height=%d\n", Width, Height);
	
	CheckDevice(this);

	LPDIRECT3DSURFACE9 baseSurface = NULL;
	if (Pool == D3DPOOL_MANAGED)
	{
		LogDebug("  Pool changed from MANAGED to DEFAULT because of DirectX9Ex migration.\n");
		
		Pool = D3DPOOL_DEFAULT;
	}
    HRESULT hr = GetD3D9Device()->CreateOffscreenPlainSurface(Width, Height, Format, Pool, &baseSurface, pSharedHandle);
	if (ppSurface && baseSurface) *ppSurface = HackerSurface9::GetDirect3DSurface9(baseSurface);    
	if (ppSurface) LogInfo("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseSurface, *ppSurface);
	
    return hr;
}

STDMETHODIMP HackerDevice9::SetRenderTarget(THIS_ DWORD RenderTargetIndex, HackerSurface9 *pRenderTarget)
{
	LogDebug("HackerDevice9::SetRenderTarget called with RenderTargetIndex=%d, pRenderTarget=%x.\n", RenderTargetIndex, pRenderTarget);
	
	CheckDevice(this);
	LPDIRECT3DSURFACE9 baseRenderTarget = replaceSurface9(pRenderTarget);
	HRESULT hr = GetD3D9Device()->SetRenderTarget(RenderTargetIndex, baseRenderTarget);
	LogInfo("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetRenderTarget(THIS_ DWORD RenderTargetIndex, HackerSurface9 **ppRenderTarget)
{
	LogDebug("HackerDevice9::GetRenderTarget called with RenderTargetIndex=%d\n", RenderTargetIndex);
	
	CheckDevice(this);

	LPDIRECT3DSURFACE9 baseSurface = 0;
    HRESULT hr = GetD3D9Device()->GetRenderTarget(RenderTargetIndex, &baseSurface);
    if (FAILED(hr) || !baseSurface)
    {
		if (!gLogDebug) LogInfo("HackerDevice9::GetRenderTarget called.\n");	
		LogInfo("  failed with hr=%x\n", hr);
		
		if (ppRenderTarget) *ppRenderTarget = 0;
        return hr;
    }
	if (ppRenderTarget && baseSurface) *ppRenderTarget = HackerSurface9::GetDirect3DSurface9(baseSurface);
	if (ppRenderTarget) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseSurface, *ppRenderTarget);
	
    return hr;
}

STDMETHODIMP HackerDevice9::SetDepthStencilSurface(THIS_ HackerSurface9 *pNewZStencil)
{
	LogDebug("HackerDevice9::SetDepthStencilSurface called with NewZStencil=%x\n", pNewZStencil);

	if (!GetD3D9Device())
	{
		LogInfo("  postponing call because device was not created yet.\n");
		
		pendingSetDepthStencilSurface = pNewZStencil;
		return S_OK;
	}

	LPDIRECT3DSURFACE9 baseStencil = replaceSurface9(pNewZStencil);
	HRESULT hr = GetD3D9Device()->SetDepthStencilSurface(baseStencil);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetDepthStencilSurface(THIS_ HackerSurface9 **ppZStencilSurface)
{
	LogDebug("HackerDevice9::GetDepthStencilSurface called.\n");
	
	CheckDevice(this);

	LPDIRECT3DSURFACE9 baseSurface = 0;
    HRESULT hr = GetD3D9Device()->GetDepthStencilSurface(&baseSurface);
    if (FAILED(hr) || !baseSurface)
    {
		if (!gLogDebug) LogInfo("HackerDevice9::GetDepthStencilSurface called.\n");	
		LogInfo("  failed with hr=%x\n", hr);
		
		if (ppZStencilSurface) *ppZStencilSurface = 0;
        return hr;
    }
	if (ppZStencilSurface) *ppZStencilSurface = HackerSurface9::GetDirect3DSurface9(baseSurface);
	if (ppZStencilSurface) LogDebug("  returns hr=%x, handle=%x, wrapper=%x\n", hr, baseSurface, *ppZStencilSurface);
	
    return hr;
}

STDMETHODIMP HackerDevice9::BeginScene(THIS)
{
	LogDebug("HackerDevice9::BeginScene called.\n");
	
	CheckDevice(this);
    HRESULT hr = GetD3D9Device()->BeginScene();
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::EndScene(THIS)
{
	LogDebug("HackerDevice9::EndScene called.\n");
	
	CheckDevice(this);
    HRESULT hr = GetD3D9Device()->EndScene();
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::Clear(THIS_ DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
	LogDebug("HackerDevice9::Clear called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->Clear(Count, pRects, Flags, Color, Z, Stencil);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetTransform(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
	LogDebug("HackerDevice9::SetTransform called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetTransform(State, pMatrix);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetTransform(THIS_ D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{
	LogDebug("HackerDevice9::GetTransform called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetTransform(State, pMatrix);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::MultiplyTransform(THIS_ D3DTRANSFORMSTATETYPE a,CONST D3DMATRIX *b)
{
	LogDebug("HackerDevice9::MultiplyTransform called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->MultiplyTransform(a, b);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetViewport(THIS_ CONST D3DVIEWPORT9* pViewport)
{
	LogDebug("HackerDevice9::SetViewport called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetViewport(pViewport);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetViewport(THIS_ D3DVIEWPORT9* pViewport)
{
	LogDebug("HackerDevice9::GetViewport called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetViewport(pViewport);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetMaterial(THIS_ CONST D3DMATERIAL9* pMaterial)
{
	LogDebug("HackerDevice9::SetMaterial called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetMaterial(pMaterial);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetMaterial(THIS_ D3DMATERIAL9* pMaterial)
{
	LogDebug("HackerDevice9::GetMaterial called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetMaterial(pMaterial);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetLight(THIS_ DWORD Index,CONST D3DLIGHT9 *Light)
{
	LogDebug("HackerDevice9::SetLight called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetLight(Index, Light);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetLight(THIS_ DWORD Index,D3DLIGHT9* Light)
{
	LogDebug("HackerDevice9::GetLight called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetLight(Index, Light);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::LightEnable(THIS_ DWORD Index,BOOL Enable)
{
	LogDebug("HackerDevice9::LightEnable called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->LightEnable(Index, Enable);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetLightEnable(THIS_ DWORD Index,BOOL* pEnable)
{
	LogDebug("HackerDevice9::GetLightEnable called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetLightEnable(Index, pEnable);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetClipPlane(THIS_ DWORD Index,CONST float* pPlane)
{
	LogDebug("HackerDevice9::SetClipPlane called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetClipPlane(Index, pPlane);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetClipPlane(THIS_ DWORD Index,float* pPlane)
{
	LogDebug("HackerDevice9::GetClipPlane called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetClipPlane(Index, pPlane);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetRenderState(THIS_ D3DRENDERSTATETYPE State,DWORD Value)
{
	LogDebug("HackerDevice9::SetRenderState called with State=%d, Value=%d\n", State, Value);

	if (!GetD3D9Device())
	{
		if (!gLogDebug) LogInfo("HackerDevice9::SetRenderState called.\n");
		LogInfo("  ignoring call because device was not created yet.\n");
		
		return S_OK;
	}

	HRESULT hr = GetD3D9Device()->SetRenderState(State, Value);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetRenderState(THIS_ D3DRENDERSTATETYPE State,DWORD* pValue)
{
	LogDebug("HackerDevice9::GetRenderState called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetRenderState(State, pValue);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::CreateStateBlock(THIS_ D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{
	LogDebug("HackerDevice9::CreateStateBlock called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->CreateStateBlock(Type, ppSB);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::BeginStateBlock(THIS)
{
	LogDebug("HackerDevice9::BeginStateBlock called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->BeginStateBlock();
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::EndStateBlock(THIS_ IDirect3DStateBlock9** ppSB)
{
	LogDebug("HackerDevice9::EndStateBlock called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->EndStateBlock(ppSB);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetClipStatus(THIS_ CONST D3DCLIPSTATUS9* pClipStatus)
{
	LogDebug("HackerDevice9::SetClipStatus called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetClipStatus(pClipStatus);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetClipStatus(THIS_ D3DCLIPSTATUS9* pClipStatus)
{
	LogDebug("HackerDevice9::GetClipStatus called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetClipStatus(pClipStatus);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetTexture(THIS_ DWORD Stage,LPDIRECT3DBASETEXTURE9* ppTexture)
{
	LogDebug("HackerDevice9::GetTexture called.\n");
	
	CheckDevice(this);

	LPDIRECT3DBASETEXTURE9 baseTexture = 0;
    HRESULT hr = GetD3D9Device()->GetTexture(Stage, &baseTexture);
    if (FAILED(hr) || !baseTexture)
    {
		if (!gLogDebug) LogInfo("HackerDevice9::GetTexture called.\n");
		LogInfo("  failed with hr=%x\n", hr);
		
		if (ppTexture) *ppTexture = 0;
        return hr;
    } 
	if (ppTexture) *ppTexture = baseTexture;
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetTexture(THIS_ DWORD Stage,LPDIRECT3DBASETEXTURE9 pTexture)
{
	LogDebug("HackerDevice9::SetTexture called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetTexture(Stage, replaceTexture9((HackerTexture9 *) pTexture));
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetTextureStageState(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
	LogDebug("HackerDevice9::GetTextureStageState called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetTextureStageState(Stage, Type, pValue);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetTextureStageState(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
	LogDebug("HackerDevice9::SetTextureStageState called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetTextureStageState(Stage, Type, Value);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetSamplerState(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
	LogDebug("HackerDevice9::GetSamplerState called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetSamplerState(Sampler, Type, pValue);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetSamplerState(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
	LogDebug("HackerDevice9::SetSamplerState called with Sampler=%d, StateType=%d, Value=%d\n", Sampler, Type, Value);

	if (!GetD3D9Device())
	{
		if (!gLogDebug) LogInfo("HackerDevice9::SetSamplerState called.\n");
		LogInfo("  ignoring call because device was not created yet.\n");
		
		return S_OK;
	}

	HRESULT hr = GetD3D9Device()->SetSamplerState(Sampler, Type, Value);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::ValidateDevice(THIS_ DWORD* pNumPasses)
{
	LogDebug("HackerDevice9::ValidateDevice called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->ValidateDevice(pNumPasses);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetPaletteEntries(THIS_ UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
	LogDebug("HackerDevice9::SetPaletteEntries called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetPaletteEntries(PaletteNumber, pEntries);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetPaletteEntries(THIS_ UINT PaletteNumber,PALETTEENTRY* pEntries)
{
	LogDebug("HackerDevice9::GetPaletteEntries called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->GetPaletteEntries(PaletteNumber, pEntries);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetCurrentTexturePalette(THIS_ UINT PaletteNumber)
{
	LogDebug("HackerDevice9::SetCurrentTexturePalette called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetCurrentTexturePalette(PaletteNumber);
}

STDMETHODIMP HackerDevice9::GetCurrentTexturePalette(THIS_ UINT *PaletteNumber)
{
	LogDebug("HackerDevice9::GetCurrentTexturePalette called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetCurrentTexturePalette( PaletteNumber);
}

STDMETHODIMP HackerDevice9::SetScissorRect(THIS_ CONST RECT* pRect)
{
	LogDebug("HackerDevice9::SetScissorRect called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetScissorRect(pRect);
}

STDMETHODIMP HackerDevice9::GetScissorRect(THIS_ RECT* pRect)
{
	LogDebug("HackerDevice9::GetScissorRect called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetScissorRect(pRect);
}

STDMETHODIMP HackerDevice9::SetSoftwareVertexProcessing(THIS_ BOOL bSoftware)
{
	LogDebug("HackerDevice9::SetSoftwareVertexProcessing called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetSoftwareVertexProcessing(bSoftware);
}

STDMETHODIMP_(BOOL) HackerDevice9::GetSoftwareVertexProcessing(THIS)
{
	LogDebug("HackerDevice9::GetSoftwareVertexProcessing called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetSoftwareVertexProcessing();
}

STDMETHODIMP HackerDevice9::SetNPatchMode(THIS_ float nSegments)
{
	LogDebug("HackerDevice9::SetNPatchMode called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetNPatchMode(nSegments);
}

STDMETHODIMP_(float) HackerDevice9::GetNPatchMode(THIS)
{
	LogDebug("HackerDevice9::GetNPatchMode called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetNPatchMode();
}

STDMETHODIMP HackerDevice9::DrawPrimitive(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
	LogDebug("HackerDevice9::DrawPrimitive called with PrimitiveType=%d, StartVertex=%d, PrimitiveCount=%d\n",
		PrimitiveType, StartVertex, PrimitiveCount);
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::DrawIndexedPrimitive(THIS_ D3DPRIMITIVETYPE Type,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
	LogDebug("HackerDevice9::DrawIndexedPrimitive called with Type=%d, BaseVertexIndex=%d, MinVertexIndex=%d, NumVertices=%d, startIndex=%d, primCount=%d\n",
		Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::DrawPrimitiveUP(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	LogDebug("HackerDevice9::DrawPrimitiveUP called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::DrawIndexedPrimitiveUP(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	LogDebug("HackerDevice9::DrawIndexedPrimitiveUP called with PrimitiveType=%d, MinVertexIndex=%d, NumVertices=%d, PrimitiveCount=%d, IndexDataFormat=%d\n",
		PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, IndexDataFormat);
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::ProcessVertices(THIS_ UINT SrcStartIndex,UINT DestIndex,UINT VertexCount, HackerVertexBuffer9 *pDestBuffer,HackerVertexDeclaration9* pVertexDecl,DWORD Flags)
{
	LogDebug("HackerDevice9::ProcessVertices called with SrcStartIndex=%d, DestIndex=%d, VertexCount=%d, Flags=%x\n",
		SrcStartIndex, DestIndex, VertexCount, Flags);
	
	CheckDevice(this);
	CheckVertexBuffer9(pDestBuffer);
	HRESULT hr = GetD3D9Device()->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, replaceVertexBuffer9(pDestBuffer), replaceVertexDeclaration9(pVertexDecl), Flags);
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::CreateVertexDeclaration(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements,HackerVertexDeclaration9** ppDecl)
{
	LogDebug("HackerDevice9::CreateVertexDeclaration called.\n");

	if (!GetD3D9Device())
	{
		LogInfo("  postponing call because device was not created yet.\n");
		
		HackerVertexDeclaration9 *wrapper = HackerVertexDeclaration9::GetDirect3DVertexDeclaration9((LPDIRECT3DVERTEXDECLARATION9) 0);
		if (pVertexElements)
			wrapper->_VertexElements = *pVertexElements;
		wrapper->pendingDevice = this;
		LogInfo("  returns handle=%x\n", wrapper);
		
		return S_OK;
	}

	LPDIRECT3DVERTEXDECLARATION9 baseVertexDeclaration = 0;
	HRESULT hr = GetD3D9Device()->CreateVertexDeclaration(pVertexElements, &baseVertexDeclaration);
	if (ppDecl && baseVertexDeclaration) *ppDecl = HackerVertexDeclaration9::GetDirect3DVertexDeclaration9(baseVertexDeclaration);
	if (ppDecl) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseVertexDeclaration, *ppDecl);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetVertexDeclaration(THIS_ HackerVertexDeclaration9* pDecl)
{
	LogDebug("HackerDevice9::SetVertexDeclaration called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetVertexDeclaration(replaceVertexDeclaration9(pDecl));
}

STDMETHODIMP HackerDevice9::GetVertexDeclaration(THIS_ HackerVertexDeclaration9** ppDecl)
{
	LogDebug("HackerDevice9::GetVertexDeclaration called.\n");
	
	CheckDevice(this);

	LPDIRECT3DVERTEXDECLARATION9 baseVertexDeclaration = 0;
	HRESULT hr = GetD3D9Device()->GetVertexDeclaration(&baseVertexDeclaration);
	if (ppDecl && baseVertexDeclaration) *ppDecl = HackerVertexDeclaration9::GetDirect3DVertexDeclaration9(baseVertexDeclaration);
	if (ppDecl) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseVertexDeclaration, *ppDecl);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetFVF(THIS_ DWORD FVF)
{
	LogDebug("HackerDevice9::SetFVF called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetFVF(FVF);
}

STDMETHODIMP HackerDevice9::GetFVF(THIS_ DWORD* pFVF)
{
	LogDebug("HackerDevice9::GetFVF called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetFVF(pFVF);
}

STDMETHODIMP HackerDevice9::CreateVertexShader(THIS_ CONST DWORD* pFunction, HackerVertexShader9 **ppShader)
{
	LogDebug("HackerDevice9::CreateVertexShader called.\n");
	
	CheckDevice(this);

	IDirect3DVertexShader9 *baseShader = 0;
	HRESULT hr = GetD3D9Device()->CreateVertexShader(pFunction, &baseShader);
	if (ppShader && baseShader) *ppShader = HackerVertexShader9::GetDirect3DVertexShader9(baseShader);
	if (ppShader) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseShader, *ppShader);
	
	return hr;
}

STDMETHODIMP HackerDevice9::SetVertexShader(THIS_ HackerVertexShader9 *pShader)
{
	LogDebug("HackerDevice9::SetVertexShader called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetVertexShader(replaceVertexShader9(pShader));
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetVertexShader(THIS_ HackerVertexShader9** ppShader)
{
	LogDebug("HackerDevice9::GetVertexShader called.\n");
	
	CheckDevice(this);
	IDirect3DVertexShader9 *baseShader = 0;
	HRESULT hr = GetD3D9Device()->GetVertexShader(&baseShader);
	if (ppShader && baseShader) *ppShader = HackerVertexShader9::GetDirect3DVertexShader9(baseShader);
	if (ppShader) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseShader, *ppShader);
	
	return hr;	
}

STDMETHODIMP HackerDevice9::SetVertexShaderConstantF(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	LogDebug("HackerDevice9::SetVertexShaderConstantF called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

STDMETHODIMP HackerDevice9::GetVertexShaderConstantF(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	LogDebug("HackerDevice9::GetVertexShaderConstantF called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

STDMETHODIMP HackerDevice9::SetVertexShaderConstantI(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	LogDebug("HackerDevice9::SetVertexShaderConstantI called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

STDMETHODIMP HackerDevice9::GetVertexShaderConstantI(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	LogDebug("HackerDevice9::GetVertexShaderConstantI called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

STDMETHODIMP HackerDevice9::SetVertexShaderConstantB(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	LogDebug("HackerDevice9::SetVertexShaderConstantB called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

STDMETHODIMP HackerDevice9::GetVertexShaderConstantB(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	LogDebug("HackerDevice9::GetVertexShaderConstantB called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

STDMETHODIMP HackerDevice9::SetStreamSource(THIS_ UINT StreamNumber, HackerVertexBuffer9 *pStreamData, UINT OffsetInBytes,UINT Stride)
{
	LogDebug("HackerDevice9::SetStreamSource called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetStreamSource(StreamNumber, replaceVertexBuffer9(pStreamData), OffsetInBytes, Stride);
}

STDMETHODIMP HackerDevice9::GetStreamSource(THIS_ UINT StreamNumber, HackerVertexBuffer9 **ppStreamData,UINT* pOffsetInBytes,UINT* pStride)
{
	LogDebug("HackerDevice9::GetStreamSource called.\n");
	
	CheckDevice(this);
	LPDIRECT3DVERTEXBUFFER9 baseVB = 0;
    HRESULT hr = GetD3D9Device()->GetStreamSource(StreamNumber, &baseVB, pOffsetInBytes, pStride);
	if (ppStreamData && baseVB) *ppStreamData = HackerVertexBuffer9::GetDirect3DVertexBuffer9(baseVB);
	if (ppStreamData) LogDebug("  returns hr=%x, handle=%x, wrapper=%x\n", hr, baseVB, *ppStreamData);
	
    return hr;
}

STDMETHODIMP HackerDevice9::SetStreamSourceFreq(THIS_ UINT StreamNumber, UINT FrequencyParameter)
{
	LogDebug("HackerDevice9::SetStreamSourceFreq called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetStreamSourceFreq(StreamNumber, FrequencyParameter);
}

STDMETHODIMP HackerDevice9::GetStreamSourceFreq(THIS_ UINT StreamNumber,UINT* pSetting)
{
	LogDebug("HackerDevice9::GetStreamSourceFreq called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetStreamSourceFreq(StreamNumber, pSetting);
}

STDMETHODIMP HackerDevice9::SetIndices(THIS_ HackerIndexBuffer9 *pIndexData)
{
	LogDebug("HackerDevice9::SetIndices called.\n");
	
	CheckDevice(this);

	HRESULT hr = GetD3D9Device()->SetIndices(replaceIndexBuffer9(pIndexData));
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetIndices(THIS_ HackerIndexBuffer9 **ppIndexData)
{
	LogDebug("HackerDevice9::GetIndices called.\n");
	
	CheckDevice(this);

	LPDIRECT3DINDEXBUFFER9 baseIB = 0;
    HRESULT hr = GetD3D9Device()->GetIndices(&baseIB);
    if (FAILED(hr) || !baseIB)
    {
		if (!gLogDebug) LogInfo("HackerDevice9::GetIndices called.\n");
		LogInfo("  failed with hr=%x\n", hr);
		
		if (ppIndexData) *ppIndexData = 0;
        return hr;
    }
	if (ppIndexData && baseIB) *ppIndexData = HackerIndexBuffer9::GetDirect3DIndexBuffer9(baseIB);
	if (ppIndexData) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseIB, *ppIndexData);
	
	return hr;
}

STDMETHODIMP HackerDevice9::CreatePixelShader(THIS_ CONST DWORD* pFunction, HackerPixelShader9 **ppShader)
{
	LogDebug("HackerDevice9::CreatePixelShader called.\n");
	
	CheckDevice(this);

	IDirect3DPixelShader9 *baseShader = 0;
	HRESULT hr = GetD3D9Device()->CreatePixelShader(pFunction, &baseShader);
	if (ppShader && baseShader) *ppShader = HackerPixelShader9::GetDirect3DPixelShader9(baseShader);
	if (ppShader) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseShader, *ppShader);
	
	return hr;

}

STDMETHODIMP HackerDevice9::SetPixelShader(THIS_ HackerPixelShader9 *pShader)
{
	LogDebug("HackerDevice9::SetPixelShader called.\n");
	
	CheckDevice(this);
	HRESULT hr = GetD3D9Device()->SetPixelShader(replacePixelShader9(pShader));
	LogDebug("  returns result=%x\n", hr);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetPixelShader(THIS_ HackerPixelShader9 **ppShader)
{
	LogDebug("HackerDevice9::GetPixelShader called.\n");
	
	CheckDevice(this);
	IDirect3DPixelShader9 *baseShader = 0;
	HRESULT hr = GetD3D9Device()->GetPixelShader(&baseShader);
	if (ppShader && baseShader) *ppShader = HackerPixelShader9::GetDirect3DPixelShader9(baseShader);
	if (ppShader) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseShader, *ppShader);
	
	return hr;	
}

STDMETHODIMP HackerDevice9::SetPixelShaderConstantF(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	LogDebug("HackerDevice9::SetPixelShaderConstantF called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

STDMETHODIMP HackerDevice9::GetPixelShaderConstantF(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	LogDebug("HackerDevice9::GetPixelShaderConstantF called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

STDMETHODIMP HackerDevice9::SetPixelShaderConstantI(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	LogDebug("HackerDevice9::SetPixelShaderConstantI called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

STDMETHODIMP HackerDevice9::GetPixelShaderConstantI(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	LogDebug("HackerDevice9::GetPixelShaderConstantI called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

STDMETHODIMP HackerDevice9::SetPixelShaderConstantB(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	LogDebug("HackerDevice9::SetPixelShaderConstantB called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

STDMETHODIMP HackerDevice9::GetPixelShaderConstantB(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	LogDebug("HackerDevice9::GetPixelShaderConstantB called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

STDMETHODIMP HackerDevice9::DrawRectPatch(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	LogDebug("HackerDevice9::DrawRectPatch called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

STDMETHODIMP HackerDevice9::DrawTriPatch(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	LogDebug("HackerDevice9::DrawTriPatch called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

STDMETHODIMP HackerDevice9::DeletePatch(THIS_ UINT Handle)
{
	LogDebug("HackerDevice9::DeletePatch called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->DeletePatch(Handle);
}

STDMETHODIMP HackerDevice9::CreateQuery(THIS_ D3DQUERYTYPE Type, HackerQuery9** ppQuery)
{
	LogDebug("HackerDevice9::CreateQuery called with Type=%d, ppQuery=%x\n", Type, ppQuery);
	
	CheckDevice(this);

	LPDIRECT3DQUERY9 baseQuery = 0;
	HRESULT hr = GetD3D9Device()->CreateQuery(Type, &baseQuery);
    if (FAILED(hr) || !baseQuery)
    {
		if (!gLogDebug) LogInfo("HackerDevice9::CreateQuery called.\n");
		LogInfo("  failed with hr=%x\n", hr);
		
		if (ppQuery) *ppQuery = 0;
        return hr;
    } 
	if (ppQuery && baseQuery) *ppQuery = HackerQuery9::GetDirect3DQuery9(baseQuery);
	if (ppQuery) LogDebug("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseQuery, *ppQuery);
	if (!ppQuery) LogDebug("  returns result=%x, handle=%x\n", hr, baseQuery);
	
	return hr;
}

STDMETHODIMP HackerDevice9::GetDisplayModeEx(THIS_ UINT iSwapChain,D3DDISPLAYMODEEX* pMode,D3DDISPLAYROTATION* pRotation)
{
	LogDebug("HackerDevice9::GetDisplayModeEx called.\n");
	
	CheckDevice(this);
	return GetD3D9Device()->GetDisplayModeEx(iSwapChain, pMode, pRotation);
}
