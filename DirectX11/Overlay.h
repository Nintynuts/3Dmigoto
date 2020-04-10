#pragma once

#include <memory>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl.h>

#include "SpriteFont.h"
#include "SpriteBatch.h"
#include "PrimitiveBatch.h"
#include "CommonStates.h"
#include "Effects.h"
#include "VertexTypes.h"
#include "om_state.h"

#include "HackerDevice.h"
#include "HackerContext.h"

#include <overlay.h>

class HackerSwapChain;

class Overlay : public OverlayBase<DirectX::SpriteFont>
{
private:
	IDXGISwapChain* mOrigSwapChain;
	ID3D11Device* mOrigDevice;
	ID3D11DeviceContext* mOrigContext;
	HackerDevice* mHackerDevice;
	HackerContext* mHackerContext;

	std::unique_ptr<DirectX::SpriteBatch> mSpriteBatch;
	std::unique_ptr<DirectX::CommonStates> mStates;
	std::unique_ptr<DirectX::BasicEffect> mEffect;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> mPrimitiveBatch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;

	// These are all state that we save away before drawing the overlay and
	// restore again afterwards. Basically everything that DirectTK
	// SimpleSprite may clobber:
	struct {
		ID3D11BlendState *pBlendState;
		FLOAT BlendFactor[4];
		UINT SampleMask;

		OMState om_state;

		D3D11_VIEWPORT pViewPorts[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		UINT RSNumViewPorts;

		ID3D11DepthStencilState *pDepthStencilState;
		UINT StencilRef;

		ID3D11RasterizerState *pRasterizerState;

		ID3D11SamplerState *samplers[1];

		D3D11_PRIMITIVE_TOPOLOGY topology;

		ID3D11InputLayout *pInputLayout;

		ID3D11VertexShader *pVertexShader;
		ID3D11ClassInstance *pVSClassInstances[256];
		UINT VSNumClassInstances;
		ID3D11Buffer *pVSConstantBuffers[1];

		ID3D11PixelShader *pPixelShader;
		ID3D11ClassInstance *pPSClassInstances[256];
		UINT PSNumClassInstances;
		ID3D11Buffer *pPSConstantBuffers[1];

		ID3D11Buffer *pVertexBuffers[1];
		UINT Strides[1];
		UINT Offsets[1];

		ID3D11Buffer *IndexBuffer;
		DXGI_FORMAT Format;
		UINT Offset;

		ID3D11ShaderResourceView *pShaderResourceViews[1];
	} state;

	virtual void SaveState() override;
	virtual void RestoreState() override;
	HRESULT InitDrawState();
	void DrawShaderInfoLine(char *type, UINT64 selectedShader, float *y, bool shader);
	virtual void DrawShaderInfoLines(float *y) override;
	virtual void DrawNotices(float *y) override;
	virtual void DrawProfiling(float *y) override;
	virtual void DrawRectangle(float x, float y, float w, float h, float r, float g, float b, float opacity) override;
	void DrawOutlinedString(DirectX::SpriteFont *font, wchar_t const *text, DirectX::XMFLOAT2 const &position, DirectX::FXMVECTOR color);

public:
	Overlay(HackerDevice *pDevice, HackerContext *pContext, IDXGISwapChain *pSwapChain);
	~Overlay();
	void DrawOverlay(void);
};
