#include "om_state.h"

void save_om_state(IDirect3DDevice9* device, struct OMState* state)
{
	DWORD i;

	// OMGetRenderTargetAndUnorderedAccessViews is a poorly designed API as
	// to use it properly to get all RTVs and UAVs we need to pass it some
	// information that we don't know. So, we have to do a few extra steps
	// to find that info.
	D3DCAPS9 caps;
	device->GetDeviceCaps(&caps);
	if (state->rtvs.size() != caps.NumSimultaneousRTs)
		state->rtvs.resize(caps.NumSimultaneousRTs);
	state->NumRTVs = 0;
	for (i = 0; i < caps.NumSimultaneousRTs; i++) {
		IDirect3DSurface9* rt = NULL;
		device->GetRenderTarget(i, &rt);
		state->rtvs[i] = rt;
		if (rt) {
			state->NumRTVs = i + 1;
		}
	}
	device->GetDepthStencilSurface(&state->dsv);
}

void restore_om_state(IDirect3DDevice9* device, struct OMState* state)
{
	UINT i;
	for (i = 0; i < state->NumRTVs; i++) {
		device->SetRenderTarget(i, state->rtvs[i]);
		if (state->rtvs[i])
			state->rtvs[i]->Release();
	}
	device->SetDepthStencilSurface(state->dsv);
	if (state->dsv)
		state->dsv->Release();
}