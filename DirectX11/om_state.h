#pragma once
#include <d3d11_1.h>

#if MIGOTO_DX == 11
extern IDXGISwapChain* last_fullscreen_swap_chain;
#endif // MIGOTO_DX == 11

struct OMState {
	UINT NumRTVs;
	ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D11DepthStencilView* dsv;
	UINT UAVStartSlot;
	UINT NumUAVs;
	ID3D11UnorderedAccessView* uavs[D3D11_PS_CS_UAV_REGISTER_COUNT];
};

void save_om_state(ID3D11DeviceContext* context, struct OMState* state);
void restore_om_state(ID3D11DeviceContext* context, struct OMState* state);
