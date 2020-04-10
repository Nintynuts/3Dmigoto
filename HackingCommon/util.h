#pragma once

#include <ctype.h>
#include <wchar.h>
#include <string>
#include <vector>
#include <map>
#include <typeinfo>

#include <d3d11_1.h>
#include <dxgi1_2.h>

#include <D3Dcompiler.h>
#include <d3d9.h>
#include <DirectXMath.h>

#include "version.h"
#include "log.h"
#include "ini_parser_lite.h"

#if MIGOTO_DX == 11
#include "DirectX11\HookedDevice.h"
#include "DirectX11\HookedContext.h"
#elif MIGOTO_DX == 9
#include "DirectX9\HookedDeviceDX9.h"
#endif // MIGOTO_DX

// Sets the threshold for warning about IniParams size. The larger IniParams is
// the more CPU -> GPU bandwidth we will require to update it, so we want to
// discourage modders from picking arbitrarily high IniParams.
//
// This threshold is somewhat arbitrary and I haven't measured how performance
// actually goes in practice, so we can tweak it as we encounter real world
// performance issues. I've chosen the page size of 4K as a starting point as
// exceeding that will likely add additional performance overheads beyond the
// bandwidth requirements (ideally we would also ensure the IniParams buffer is
// aligned to a page boundary).
//
// If a shaderhacker wants more than 1024 (256x4) IniParams they should
// probably think about using a different storage means anyway, since IniParams
// has other problems such as no meaningful names, no namespacing, etc.
const int INI_PARAMS_SIZE_WARNING = 256;

// -----------------------------------------------------------------------------------------------

bool check_interface_supported(IUnknown *unknown, REFIID riid);
void analyse_iunknown(IUnknown *unknown);

// -----------------------------------------------------------------------------------------------

void install_crash_handler(int level);
