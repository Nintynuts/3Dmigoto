#pragma once

#include "HookedDeviceDX9.h"

#include <type_name.h>

static const char* type_name_dx9(IUnknown* object)
{
	IDirect3DDevice9Ex* device;

	// Seems that not even try / catch is safe in all cases of this
	// (grumble grumble poorly designed grumble...). The only cases where
	// we should be called on an object without type information is while
	// hooking the device and/or context, so check if it is one of those
	// cases:

	device = lookup_hooked_device_dx9((IDirect3DDevice9Ex*)object);
	if (device)
		return "Hooked_IDirect3DDevice9";

	return type_name(object);
}
#define type_name type_name_dx9