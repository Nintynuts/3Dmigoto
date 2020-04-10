#pragma once
#include <d3d9.h>
#include <vector>

struct OMState {
	UINT NumRTVs;
	std::vector<IDirect3DSurface9*> rtvs;
	IDirect3DSurface9* dsv;
};

void save_om_state(IDirect3DDevice9* device, struct OMState* state);
void restore_om_state(IDirect3DDevice9* device, struct OMState* state);