#pragma once

#include <windows.h>
#include <d3d9.h>
#include <IDirect3DUnknown.h>
#include <PointerSet.h>

class Hacker9;
class HackerDevice9;
class HackerSwapChain9;
class HackerSurface9;
class HackerVertexDeclaration9;
class HackerTexture9;
class HackerVertexBuffer9;
class HackerIndexBuffer9;
class HackerQuery9;
class HackerVertexShader9;
class HackerPixelShader9;

namespace Func {
	typedef HRESULT(WINAPI* D3DCREATE)(UINT, LPDIRECT3D9EX* ppD3D);
	typedef int (WINAPI* D3DPERF_BeginEvent)(D3DCOLOR color, LPCWSTR name);
	typedef int (WINAPI* D3DPERF_EndEvent)(void);
	typedef DWORD(WINAPI* D3DPERF_GetStatus)(void);
	typedef BOOL(WINAPI* D3DPERF_QueryRepeatFrame)(void);
	typedef void (WINAPI* D3DPERF_SetMarker)(D3DCOLOR color, LPCWSTR name);
	typedef void (WINAPI* D3DPERF_SetOptions)(DWORD options);
	typedef void (WINAPI* D3DPERF_SetRegion)(D3DCOLOR color, LPCWSTR name);
	typedef void (WINAPI* DebugSetLevel)(int a1, int a2);
	typedef void (WINAPI* DebugSetMute)(int a);
	typedef void* (WINAPI* Direct3DShaderValidatorCreate9)(void);
	typedef void (WINAPI* PSGPError)(void* D3DFE_PROCESSVERTICES, int PSGPERRORID, unsigned int a);
	typedef void (WINAPI* PSGPSampleTexture)(void* D3DFE_PROCESSVERTICES, unsigned int a, float(* const b)[4], unsigned int c, float(* const d)[4]);
}

class Hacker9Base : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet	m_List;

	Hacker9Base(IUnknown* pUnk)
		: IDirect3DUnknown(pUnk) {}

	/*** IDirect3DUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** Hacker9 methods ***/
	STDMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction);
	STDMETHOD_(UINT, GetAdapterCount)(THIS) PURE;
	STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) PURE;
	STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter, D3DFORMAT Format) PURE;
	STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) PURE;
	STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter, D3DDISPLAYMODE* pMode) PURE;
	STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) PURE;
	STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) PURE;
	STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) PURE;
	STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) PURE;
	STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) PURE;
	STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) PURE;
	STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter) PURE;
	STDMETHOD(CreateDevice)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, HackerDevice9** ppReturnedDeviceInterface) PURE;
};

class Hacker9 : public Hacker9Base
{
public:
	Hacker9(LPDIRECT3D9EX pD3D);
	static Hacker9* GetDirect3D(LPDIRECT3D9EX pD3D);
	__forceinline LPDIRECT3D9EX GetDirect3D9() { return (LPDIRECT3D9EX)m_pUnk; }

	/*** Hacker9 methods ***/
	STDMETHOD_(UINT, GetAdapterCount)(THIS);
	STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier);
	STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter, D3DFORMAT Format);
	STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode);
	STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter, D3DDISPLAYMODE* pMode);
	STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
	STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
	STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels);
	STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
	STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
	STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps);
	STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter);
	STDMETHOD(CreateDevice)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, HackerDevice9** ppReturnedDeviceInterface);

	/*** Hacker9Ex methods ***/
	STDMETHOD_(UINT, GetAdapterModeCountEx)(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter);
	STDMETHOD(EnumAdapterModesEx)(THIS_ UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode);
	STDMETHOD(GetAdapterDisplayModeEx)(THIS_ UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation);
	STDMETHOD(CreateDeviceEx)(THIS_ UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, HackerDevice9** ppReturnedDeviceInterface);
	STDMETHOD(GetAdapterLUID)(THIS_ UINT Adapter, LUID* pLUID);
};

class HackerDevice9 : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet	 m_List;
	// Creation parameters
	HANDLE _CreateThread;
	LPDIRECT3D9EX _pD3D;
	UINT _Adapter;
	D3DDEVTYPE _DeviceType;
	HWND _hFocusWindow;
	DWORD _BehaviorFlags;
	D3DDISPLAYMODEEX _pFullscreenDisplayMode;
	D3DPRESENT_PARAMETERS _pPresentationParameters;
	HackerSurface9* pendingCreateDepthStencilSurface;
	HackerSurface9* pendingSetDepthStencilSurface;

	HackerDevice9(LPDIRECT3DDEVICE9EX pDevice);
	static HackerDevice9* GetDirect3DDevice(LPDIRECT3DDEVICE9EX pDevice);
	__forceinline LPDIRECT3DDEVICE9EX GetD3D9Device() { return (LPDIRECT3DDEVICE9EX)m_pUnk; }

	/*** IDirect3DUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** HackerDevice9 methods ***/
	STDMETHOD(TestCooperativeLevel)(THIS);
	STDMETHOD_(UINT, GetAvailableTextureMem)(THIS);
	STDMETHOD(EvictManagedResources)(THIS);
	STDMETHOD(GetDirect3D)(THIS_ Hacker9** ppD3D9);
	STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps);
	STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain, D3DDISPLAYMODE* pMode);
	STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS* pParameters);
	STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, HackerSurface9* pCursorBitmap);
	STDMETHOD_(void, SetCursorPosition)(THIS_ int X, int Y, DWORD Flags);
	STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow);
	STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, HackerSwapChain9** pSwapChain);
	STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain, HackerSwapChain9** pSwapChain);
	STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS);
	STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, HackerSurface9** ppBackBuffer);
	STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus);
	STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs);
	STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain, D3DGAMMARAMP* pRamp);
	STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HackerTexture9** ppTexture, HANDLE* pSharedHandle);
	STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle);
	STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle);
	STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, HackerVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle);
	STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, HackerIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle);
	STDMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, HackerSurface9** ppSurface, HANDLE* pSharedHandle);
	STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, HackerSurface9** ppSurface, HANDLE* pSharedHandle);
	STDMETHOD(UpdateSurface)(THIS_ HackerSurface9* pSourceSurface, CONST RECT* pSourceRect, HackerSurface9* pDestinationSurface, CONST POINT* pDestPoint);
	STDMETHOD(UpdateTexture)(THIS_ LPDIRECT3DBASETEXTURE9 pSourceTexture, LPDIRECT3DBASETEXTURE9 pDestinationTexture);
	STDMETHOD(GetRenderTargetData)(THIS_ HackerSurface9* pRenderTarget, HackerSurface9* pDestSurface);
	STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain, HackerSurface9* pDestSurface);
	STDMETHOD(StretchRect)(THIS_ HackerSurface9* pSourceSurface, CONST RECT* pSourceRect, HackerSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
	STDMETHOD(ColorFill)(THIS_ HackerSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color);
	STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, HackerSurface9** ppSurface, HANDLE* pSharedHandle);
	STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex, HackerSurface9* pRenderTarget);
	STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex, HackerSurface9** ppRenderTarget);
	STDMETHOD(SetDepthStencilSurface)(THIS_ HackerSurface9* pNewZStencil);
	STDMETHOD(GetDepthStencilSurface)(THIS_ HackerSurface9** ppZStencilSurface);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix);
	STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*);
	STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport);
	STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport);
	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial);
	STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial);
	STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9*);
	STDMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT9*);
	STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable);
	STDMETHOD(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable);
	STDMETHOD(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane);
	STDMETHOD(GetClipPlane)(THIS_ DWORD Index, float* pPlane);
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value);
	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue);
	STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB);
	STDMETHOD(BeginStateBlock)(THIS);
	STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB);
	STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus);
	STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus);
	STDMETHOD(GetTexture)(THIS_ DWORD Stage, LPDIRECT3DBASETEXTURE9* ppTexture);
	STDMETHOD(SetTexture)(THIS_ DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture);
	STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
	STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue);
	STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses);
	STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries);
	STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries);
	STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber);
	STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT* PaletteNumber);
	STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect);
	STDMETHOD(GetScissorRect)(THIS_ RECT* pRect);
	STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware);
	STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS);
	STDMETHOD(SetNPatchMode)(THIS_ float nSegments);
	STDMETHOD_(float, GetNPatchMode)(THIS);
	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, HackerVertexBuffer9* pDestBuffer, HackerVertexDeclaration9* pVertexDecl, DWORD Flags);
	STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements, HackerVertexDeclaration9** ppDecl);
	STDMETHOD(SetVertexDeclaration)(THIS_ HackerVertexDeclaration9* pDecl);
	STDMETHOD(GetVertexDeclaration)(THIS_ HackerVertexDeclaration9** ppDecl);
	STDMETHOD(SetFVF)(THIS_ DWORD FVF);
	STDMETHOD(GetFVF)(THIS_ DWORD* pFVF);
	STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction, HackerVertexShader9** ppShader);
	STDMETHOD(SetVertexShader)(THIS_ HackerVertexShader9* pShader);
	STDMETHOD(GetVertexShader)(THIS_ HackerVertexShader9** ppShader);
	STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, HackerVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);
	STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, HackerVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride);
	STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT Setting);
	STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber, UINT* pSetting);
	STDMETHOD(SetIndices)(THIS_ HackerIndexBuffer9* pIndexData);
	STDMETHOD(GetIndices)(THIS_ HackerIndexBuffer9** ppIndexData);
	STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, HackerPixelShader9** ppShader);
	STDMETHOD(SetPixelShader)(THIS_ HackerPixelShader9* pShader);
	STDMETHOD(GetPixelShader)(THIS_ HackerPixelShader9** ppShader);
	STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	STDMETHOD(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	STDMETHOD(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	STDMETHOD(DeletePatch)(THIS_ UINT Handle);
	STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type, HackerQuery9** ppQuery);

	/*** HackerDevice9Ex methods ***/
	STDMETHOD(GetDisplayModeEx)(THIS_ UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation);
};

class HackerSwapChain9 : public IDirect3DUnknown
{
public:
	//    HackerDevice9* m_pDevice;
	static ThreadSafePointerSet	   m_List;
	// Postponed creation parameters.
	bool pendingGetSwapChain;
	UINT _SwapChain;
	HackerDevice9* pendingDevice;

	HackerSwapChain9(LPDIRECT3DSWAPCHAIN9EX pSwapChain, HackerDevice9* pDevice);

	/*** IDirect3DUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	static HackerSwapChain9* GetSwapChain(LPDIRECT3DSWAPCHAIN9EX pSwapChain, HackerDevice9* pDevice);
	inline LPDIRECT3DSWAPCHAIN9EX GetSwapChain9() { return (LPDIRECT3DSWAPCHAIN9EX)m_pUnk; }

	/*** HackerSwapChain9 methods ***/
	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
	STDMETHOD(GetFrontBufferData)(THIS_ HackerSurface9* pDestSurface);
	STDMETHOD(GetBackBuffer)(THIS_ UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, HackerSurface9** ppBackBuffer);
	STDMETHOD(GetRasterStatus)(THIS_ D3DRASTER_STATUS* pRasterStatus);
	STDMETHOD(GetDisplayMode)(THIS_ D3DDISPLAYMODE* pMode);
	STDMETHOD(GetDevice)(THIS_ HackerDevice9** ppDevice);
	STDMETHOD(GetPresentParameters)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);

	/*** HackerSwapChain9Ex methods ***/
	STDMETHOD(GetDisplayModeEx)(THIS_ D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation);
};

class HackerSurface9 : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet m_List;
	// Delayed creation parameters from device.
	int magic;
	UINT _Width;
	UINT _Height;
	D3DFORMAT _Format;
	D3DMULTISAMPLE_TYPE _MultiSample;
	DWORD _MultisampleQuality;
	BOOL _Discard;
	HackerDevice9* _Device;
	// Delayed creation parameters from texture.
	UINT _Level;
	HackerTexture9* _Texture;
	bool pendingGetSurfaceLevel;

	HackerSurface9(LPDIRECT3DSURFACE9 pSurface);
	static HackerSurface9* GetDirect3DSurface9(LPDIRECT3DSURFACE9 pSurface);
	__forceinline LPDIRECT3DSURFACE9 GetD3DSurface9() { return (LPDIRECT3DSURFACE9)m_pUnk; }

	/*** IUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** HackerResource9 methods ***/
	STDMETHOD(GetDevice)(THIS_ HackerDevice9** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	STDMETHOD_(DWORD, GetPriority)(THIS);
	STDMETHOD_(void, PreLoad)(THIS);
	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS);

	/*** HackerSurface9 methods ***/
	STDMETHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer);
	STDMETHOD(GetDesc)(THIS_ D3DSURFACE_DESC* pDesc);
	STDMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	STDMETHOD(UnlockRect)(THIS);
	STDMETHOD(GetDC)(THIS_ HDC* phdc);
	STDMETHOD(ReleaseDC)(THIS_ HDC hdc);
};

class HackerVertexDeclaration9 : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet m_List;
	int magic;
	// Delayed creation parameters.
	D3DVERTEXELEMENT9 _VertexElements;
	HackerDevice9* pendingDevice;
	bool pendingCreateVertexDeclaration;

	HackerVertexDeclaration9(LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration);
	static HackerVertexDeclaration9* GetDirect3DVertexDeclaration9(LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration);
	__forceinline LPDIRECT3DVERTEXDECLARATION9 GetD3DVertexDeclaration9() { return (LPDIRECT3DVERTEXDECLARATION9)m_pUnk; }

	/*** IUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** HackerVertexDeclaration9 methods ***/
	STDMETHOD(GetDevice)(THIS_ HackerDevice9** ppDevice);
	STDMETHOD(GetDeclaration)(THIS_ D3DVERTEXELEMENT9* pElement, UINT* pNumElements);
};

class HackerTexture9 : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet m_List;
	// Delayed creation parameters.
	int magic;
	UINT _Width;
	UINT _Height;
	UINT _Levels;
	DWORD _Usage;
	D3DFORMAT _Format;
	D3DPOOL _Pool;
	HackerDevice9* _Device;
	bool pendingCreateTexture;
	// Delayed lock/unlock.
	DWORD _Flags;
	UINT _Level;
	char* _Buffer;
	bool pendingLockUnlock;

	HackerTexture9(LPDIRECT3DTEXTURE9 pTexture);
	static HackerTexture9* GetDirect3DVertexDeclaration9(LPDIRECT3DTEXTURE9 pTexture);
	__forceinline LPDIRECT3DTEXTURE9 GetD3DTexture9() { return (LPDIRECT3DTEXTURE9)m_pUnk; }

	/*** IUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** HackerBaseTexture9 methods ***/
	STDMETHOD(GetDevice)(THIS_ HackerDevice9** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	STDMETHOD_(DWORD, GetPriority)(THIS);
	STDMETHOD_(void, PreLoad)(THIS);
	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS);
	STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew);
	STDMETHOD_(DWORD, GetLOD)(THIS);
	STDMETHOD_(DWORD, GetLevelCount)(THIS);
	STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType);
	STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS);
	STDMETHOD_(void, GenerateMipSubLevels)(THIS);
	STDMETHOD(GetLevelDesc)(THIS_ UINT Level, D3DSURFACE_DESC* pDesc);
	STDMETHOD(GetSurfaceLevel)(THIS_ UINT Level, HackerSurface9** ppSurfaceLevel);
	STDMETHOD(LockRect)(THIS_ UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	STDMETHOD(UnlockRect)(THIS_ UINT Level);
	STDMETHOD(AddDirtyRect)(THIS_ CONST RECT* pDirtyRect);
};

class HackerVertexBuffer9 : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet m_List;
	int magic;
	// Delayed creation parameters.
	UINT _Length;
	DWORD _Usage;
	DWORD _FVF;
	D3DPOOL _Pool;
	HackerDevice9* _Device;
	bool pendingCreateVertexBuffer;
	// Delayed buffer lock/unlock.
	char* _Buffer;
	DWORD _Flags;
	bool pendingLockUnlock;

	HackerVertexBuffer9(LPDIRECT3DVERTEXBUFFER9 pVertexBuffer);
	static HackerVertexBuffer9* GetDirect3DVertexBuffer9(LPDIRECT3DVERTEXBUFFER9 pVertexBuffer);
	__forceinline LPDIRECT3DVERTEXBUFFER9 GetD3DVertexBuffer9() { return (LPDIRECT3DVERTEXBUFFER9)m_pUnk; }

	/*** IUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** HackerResource9 methods ***/
	STDMETHOD(GetDevice)(THIS_ HackerDevice9** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	STDMETHOD_(DWORD, GetPriority)(THIS);
	STDMETHOD_(void, PreLoad)(THIS);
	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS);

	/*** HackerResource9 methods ***/
	STDMETHOD(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
	STDMETHOD(Unlock)(THIS);
	STDMETHOD(GetDesc)(THIS_ D3DVERTEXBUFFER_DESC* pDesc);
};

class HackerIndexBuffer9 : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet m_List;
	int magic;
	// Delayed creation parameters.
	UINT _Length;
	DWORD _Usage;
	D3DFORMAT _Format;
	D3DPOOL _Pool;
	HackerDevice9* _Device;
	bool pendingCreateIndexBuffer;
	// Delayed buffer lock/unlock.
	char* _Buffer;
	DWORD _Flags;
	bool pendingLockUnlock;

	HackerIndexBuffer9(LPDIRECT3DINDEXBUFFER9 pIndexBuffer);
	static HackerIndexBuffer9* GetDirect3DIndexBuffer9(LPDIRECT3DINDEXBUFFER9 pIndexBuffer);
	__forceinline LPDIRECT3DINDEXBUFFER9 GetD3DIndexBuffer9() { return (LPDIRECT3DINDEXBUFFER9)m_pUnk; }

	/*** IUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** HackerResource9 methods ***/
	STDMETHOD(GetDevice)(THIS_ HackerDevice9** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	STDMETHOD_(DWORD, GetPriority)(THIS);
	STDMETHOD_(void, PreLoad)(THIS);
	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS);

	/*** HackerResource9 methods ***/
	STDMETHOD(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags);
	STDMETHOD(Unlock)(THIS);
	STDMETHOD(GetDesc)(THIS_ D3DINDEXBUFFER_DESC* pDesc);
};

class HackerQuery9 : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet m_List;
	int magic;

	HackerQuery9(LPDIRECT3DQUERY9 pQuery);
	static HackerQuery9* GetDirect3DQuery9(LPDIRECT3DQUERY9 pQuery);
	__forceinline LPDIRECT3DQUERY9 GetD3DQuery9() { return (LPDIRECT3DQUERY9)m_pUnk; }

	/*** IUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** HackerQuery9 methods ***/
	STDMETHOD(GetDevice)(THIS_ HackerDevice9** ppDevice);
	STDMETHOD_(D3DQUERYTYPE, GetType)(THIS);
	STDMETHOD_(DWORD, GetDataSize)(THIS);
	STDMETHOD(Issue)(THIS_ DWORD dwIssueFlags);
	STDMETHOD(GetData)(THIS_ void* pData, DWORD dwSize, DWORD dwGetDataFlags);
};

class HackerVertexShader9 : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet m_List;
	int magic;

	HackerVertexShader9(LPDIRECT3DVERTEXSHADER9 pVS);
	static HackerVertexShader9* GetDirect3DVertexShader9(LPDIRECT3DVERTEXSHADER9 pVS);
	__forceinline LPDIRECT3DVERTEXSHADER9 GetD3DVertexShader9() { return (LPDIRECT3DVERTEXSHADER9)m_pUnk; }

	/*** IUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** HackerVertexShader9 methods ***/
	STDMETHOD(GetDevice)(THIS_ HackerDevice9** ppDevice);
	STDMETHOD(GetFunction)(THIS_ void*, UINT* pSizeOfData);
};

class HackerPixelShader9 : public IDirect3DUnknown
{
public:
	static ThreadSafePointerSet m_List;
	int magic;

	HackerPixelShader9(LPDIRECT3DPIXELSHADER9 pVS);
	static HackerPixelShader9* GetDirect3DPixelShader9(LPDIRECT3DPIXELSHADER9 pVS);
	__forceinline LPDIRECT3DPIXELSHADER9 GetD3DPixelShader9() { return (LPDIRECT3DPIXELSHADER9)m_pUnk; }

	/*** IUnknown methods ***/
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** HackerPixelShader9 methods ***/
	STDMETHOD(GetDevice)(THIS_ HackerDevice9** ppDevice);
	STDMETHOD(GetFunction)(THIS_ void*, UINT* pSizeOfData);
};
