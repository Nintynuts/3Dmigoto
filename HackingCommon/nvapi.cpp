#include "nvapi.h"
#include "log.h"

#include <windows.h>

// -----------------------------------------------------------------------------------------------
// We used to call nvapi_QueryInterface directly, however that puts nvapi.dll /
// nvapi64.dll in our dependencies list and the Windows dynamic linker will
// refuse to load us if that doesn't exist, which is a problem on AMD or Intel
// hardware and a problem for some of our users who are interested in 3DMigoto
// for reasons beyond 3D Vision modding. The way nvapi is supposed to work is
// that we call functions in the nvapi *static* library and it will try to load
// the dynamic library, and gracefully fail if it could not, but directly
// calling nvapi_QueryInterface thwarts that because that call does not come
// from the static library - it is how the static library calls into the
// dynamic library.
//
// Instead we now load nvapi.dll at runtime in the same way that the static
// library does, failing gracefully if we could not.

static HMODULE nvDLL;
static bool nvapi_failed = false;
typedef NvAPI_Status* (__cdecl* nvapi_QueryInterfaceType)(unsigned int offset);
static nvapi_QueryInterfaceType nvapi_QueryInterfacePtr;

void NvAPIOverride()
{
	static bool warned = false;

	if (nvapi_failed)
		return;

	if (!nvDLL) {
		// Use GetModuleHandleEx instead of GetModuleHandle to bump the
		// refcount on our nvapi wrapper since we are storing a
		// function pointer from the library. This is important, since
		// if we aren't running on an nvidia system the nvapi static
		// library will unload the dynamic library, which would lead to
		// a crash when we later try to call the NvAPI_QueryInterface
		// function pointer.
		GetModuleHandleEx(0, L"nvapi64.dll", &nvDLL);
		if (!nvDLL) {
			GetModuleHandleEx(0, L"nvapi.dll", &nvDLL);
		}
		if (!nvDLL) {
			LogInfo("Can't get nvapi handle\n");
			nvapi_failed = true;
			return;
		}
	}
	if (!nvapi_QueryInterfacePtr) {
		nvapi_QueryInterfacePtr = (nvapi_QueryInterfaceType)GetProcAddress(nvDLL, "nvapi_QueryInterface");
		LogDebug("nvapi_QueryInterfacePtr @ 0x%p\n", nvapi_QueryInterfacePtr);
		if (!nvapi_QueryInterfacePtr) {
			LogInfo("Unable to call NvAPI_QueryInterface\n");
			nvapi_failed = true;
			return;
		}
	}

	// One shot, override custom settings.
	intptr_t ret = (intptr_t)nvapi_QueryInterfacePtr(0xb03bb03b);
	if (!warned && (ret & 0xffffffff) != 0xeecc34ab) {
		LogInfo("  overriding NVAPI wrapper failed.\n");
		warned = true;
	}
}