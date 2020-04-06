#pragma once

#include "DirectX11\HookedDevice.h"
#include "DirectX11\HookedContext.h"
#include "type_name.h"

static const char* type_name_dx11(IUnknown* object)
{
	ID3D11Device1* device;
	ID3D11DeviceContext1* context;

	// Seems that not even try / catch is safe in all cases of this
	// (grumble grumble poorly designed grumble...). The only cases where
	// we should be called on an object without type information is while
	// hooking the device and/or context, so check if it is one of those
	// cases:

	device = lookup_hooked_device((ID3D11Device1*)object);
	if (device)
		return "Hooked_ID3D11Device";
	context = lookup_hooked_context((ID3D11DeviceContext1*)object);
	if (context)
		return "Hooked_ID3D11DeviceContext";

	return type_name(object);
}

#define type_name type_name_dx11