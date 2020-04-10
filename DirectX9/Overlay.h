#pragma once
#include <memory>
#include "D3DFont.h"
#include <gdiplus.h>
#include <DirectXMath.h>
#include "CommandList.h"

// Forward references required because of circular references from the
// other 'Hacker' objects.
#include <d3d9.h>
namespace D3D9Wrapper {
	class IDirect3DDevice9;
	class IDirect3DShader9;
}

#include <overlay.h>

class Overlay : public OverlayBase<CD3DFont>
{
private:
	D3D9Wrapper::IDirect3DDevice9 *mHackerDevice;
	::IDirect3DStateBlock9 *saved_state_block;
	virtual void SaveState() override;
	virtual void RestoreState() override;
	HRESULT InitDrawState(::IDirect3DSwapChain9 *pSwapChain = NULL);
	void DrawShaderInfoLine(wchar_t *type, UINT64 selectedShader, float *y);
	void DrawShaderInfoLine(wchar_t *type, D3D9Wrapper::IDirect3DShader9 *selectedShader, float *y);
	void DrawShaderInfoLine(wchar_t *osdString, float *y);
	virtual void DrawShaderInfoLines(float *y) override;
	virtual void DrawNotices(float *y) override;
	virtual void DrawProfiling(float *y) override;
	virtual void DrawRectangle(float x, float y, float w, float h, float r, float g, float b, float opacity) override;
	void DrawOutlinedString(CD3DFont *font, wchar_t const *text, ::D3DXVECTOR2 const &position, DWORD color);

	ULONG migotoResourceCount;

public:
	Overlay(D3D9Wrapper::IDirect3DDevice9 *pDevice);
	~Overlay();
	void DrawOverlay(CachedStereoValues *cachedStereoValues = NULL, ::IDirect3DSwapChain9 *pSwapChain = NULL);
	void Resize(UINT Width, UINT Height);
	ULONG ReferenceCount();
};
