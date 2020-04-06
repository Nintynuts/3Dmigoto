
HackerTexture9::HackerTexture9(LPDIRECT3DTEXTURE9 pTexture)
    : IDirect3DUnknown((IUnknown*) pTexture),
	pendingCreateTexture(false),
	pendingLockUnlock(false),
	magic(0x7da43feb)
{
}

HackerTexture9* HackerTexture9::GetDirect3DVertexDeclaration9(LPDIRECT3DTEXTURE9 pTexture)
{
    HackerTexture9* p = (HackerTexture9*) m_List.GetDataPtr(pTexture);
    if (p == NULL)
    {
        p = new HackerTexture9(pTexture);
		if (pTexture) m_List.AddMember(pTexture, p);
    }
    return p;
}

STDMETHODIMP_(ULONG) HackerTexture9::AddRef(THIS)
{
	++m_ulRef;
	return m_pUnk->AddRef();
}

STDMETHODIMP_(ULONG) HackerTexture9::Release(THIS)
{
	LogDebug("HackerTexture9::Release handle=%x, counter=%d, this=%x\n", m_pUnk, m_ulRef, this);
	
    ULONG ulRef = m_pUnk ? m_pUnk->Release() : 0;
	LogDebug("  internal counter = %d\n", ulRef);
	
	--m_ulRef;

    if (ulRef == 0)
    {
		if (!gLogDebug) LogInfo("HackerTexture9::Release handle=%x, counter=%d, internal counter = %d\n", m_pUnk, m_ulRef, ulRef);
		LogInfo("  deleting self\n");
		
        if (m_pUnk) m_List.DeleteMember(m_pUnk); 
		m_pUnk = 0;
        delete this;
        return 0L;
    }
    return ulRef;
}

STDMETHODIMP HackerTexture9::GetDevice(THIS_ HackerDevice9** ppDevice)
{
	LogDebug("HackerTexture9::GetDevice called\n");
	
	CheckTexture9(this);
	IDirect3DDevice9 *origDevice;
	HRESULT hr = GetD3DTexture9()->GetDevice(&origDevice);
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

STDMETHODIMP HackerTexture9::SetPrivateData(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags)
{
	LogDebug("HackerTexture9::SetPrivateData called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

STDMETHODIMP HackerTexture9::GetPrivateData(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData)
{
	LogDebug("HackerTexture9::GetPrivateData called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->GetPrivateData(refguid, pData, pSizeOfData);
}

STDMETHODIMP HackerTexture9::FreePrivateData(THIS_ REFGUID refguid)
{
	LogDebug("HackerTexture9::FreePrivateData called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->FreePrivateData(refguid);
}

STDMETHODIMP_(DWORD) HackerTexture9::SetPriority(THIS_ DWORD PriorityNew)
{
	LogDebug("HackerTexture9::SetPriority called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->SetPriority(PriorityNew);
}

STDMETHODIMP_(DWORD) HackerTexture9::GetPriority(THIS)
{
	LogDebug("HackerTexture9::GetPriority called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->GetPriority();
}

STDMETHODIMP_(void) HackerTexture9::PreLoad(THIS)
{
	LogDebug("HackerTexture9::PreLoad called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->PreLoad();
}

STDMETHODIMP_(D3DRESOURCETYPE) HackerTexture9::GetType(THIS)
{
	LogDebug("HackerTexture9::GetType called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->GetType();
}

STDMETHODIMP_(DWORD) HackerTexture9::SetLOD(THIS_ DWORD LODNew)
{
	LogDebug("HackerTexture9::SetLOD called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->SetLOD(LODNew);
}

STDMETHODIMP_(DWORD) HackerTexture9::GetLOD(THIS)
{
	LogDebug("HackerTexture9::GetLOD called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->GetLOD();
}

STDMETHODIMP_(DWORD) HackerTexture9::GetLevelCount(THIS)
{
	LogDebug("HackerTexture9::GetLevelCount called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->GetLevelCount();
}

STDMETHODIMP HackerTexture9::SetAutoGenFilterType(THIS_ D3DTEXTUREFILTERTYPE FilterType)
{
	LogDebug("HackerTexture9::SetAutoGenFilterType called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->SetAutoGenFilterType(FilterType);
}

STDMETHODIMP_(D3DTEXTUREFILTERTYPE) HackerTexture9::GetAutoGenFilterType(THIS)
{
	LogDebug("HackerTexture9::GetAutoGenFilterType called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->GetAutoGenFilterType();
}

STDMETHODIMP_(void) HackerTexture9::GenerateMipSubLevels(THIS)
{
	LogDebug("HackerTexture9::GenerateMipSubLevels called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->GenerateMipSubLevels();
}

STDMETHODIMP HackerTexture9::GetLevelDesc(THIS_ UINT Level,D3DSURFACE_DESC *pDesc)
{
	LogDebug("HackerTexture9::GetLevelDesc called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->GetLevelDesc(Level, pDesc);
}

STDMETHODIMP HackerTexture9::GetSurfaceLevel(THIS_ UINT Level,HackerSurface9** ppSurfaceLevel)
{
	LogDebug("HackerTexture9::GetSurfaceLevel called\n");
	

	if (!GetD3DTexture9())
	{
		LogInfo("  postponing call because texture was not created yet.\n");
		
		HackerSurface9 *wrapper = HackerSurface9::GetDirect3DSurface9((LPDIRECT3DSURFACE9) 0);
		wrapper->_Level = Level;
		wrapper->_Texture = this;
		wrapper->pendingGetSurfaceLevel = true;
		*ppSurfaceLevel = wrapper;
		LogInfo("  returns handle=%x\n", wrapper);
		
		return S_OK;
	}

	IDirect3DSurface9 *baseSurfaceLevel = 0;
	HRESULT hr = GetD3DTexture9()->GetSurfaceLevel(Level, &baseSurfaceLevel);
	if (ppSurfaceLevel && baseSurfaceLevel) *ppSurfaceLevel = HackerSurface9::GetDirect3DSurface9(baseSurfaceLevel);
	if (ppSurfaceLevel) LogInfo("  returns result=%x, handle=%x, wrapper=%x\n", hr, baseSurfaceLevel, *ppSurfaceLevel);
	
	return hr;
}

STDMETHODIMP HackerTexture9::LockRect(THIS_ UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
{
	LogDebug("HackerTexture9::LockRect called with Level=%d, Rect=l:%d,u:%d,r:%d,b:%d\n", Level,
		pRect ? pRect->left : 0, pRect ? pRect->top : 0, pRect ? pRect->right : 0, pRect ? pRect->bottom : 0);
	
	if (!GetD3DTexture9())
	{
		if (!gLogDebug) LogInfo("HackerTexture9::LockRect called\n");
		LogInfo("  postponing call because texture was not created yet.\n");
		
		if (!pendingLockUnlock)
		{
			_Flags = Flags;
			_Level = Level;
			_Buffer = new char[_Width*_Height*4];
			pendingLockUnlock = true;
		}
		if (pLockedRect)
		{
			pLockedRect->Pitch = _Width*4;
			pLockedRect->pBits = _Buffer;
		}
		return S_OK;
	}

	return GetD3DTexture9()->LockRect(Level, pLockedRect, pRect, Flags);
}

STDMETHODIMP HackerTexture9::UnlockRect(THIS_ UINT Level)
{
	LogDebug("HackerTexture9::UnlockRect called\n");

	if (!GetD3DTexture9())
	{
		if (!gLogDebug) LogInfo("HackerTexture9::UnlockRect called\n");
		LogInfo("  postponing call because texture was not created yet.\n");
		
		return S_OK;
	}

	return GetD3DTexture9()->UnlockRect(Level);
}

STDMETHODIMP HackerTexture9::AddDirtyRect(THIS_ CONST RECT* pDirtyRect)
{
	LogDebug("HackerTexture9::AddDirtyRect called\n");
	
	CheckTexture9(this);
	return GetD3DTexture9()->AddDirtyRect(pDirtyRect);
}
