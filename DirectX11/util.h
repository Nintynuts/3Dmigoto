#pragma once

#include <stdlib.h>
#include <DirectXMath.h>

#include <d3d11_4.h>

#include <version.h>
#include <log.h>
#include "crc32c.h"

#include "../D3D_Shaders/stdafx.h"

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

// Create hash code for textures or buffers.  

// Wrapped in try/catch because it can crash in Dirt Rally,
// because of noncontiguous or non-mapped memory for the texture.  Not sure this
// is the best strategy.

// Now switching to use crc32_append instead of fnv_64_buf for performance. This
// implementation of crc32c uses the SSE 4.2 instructions in the CPU to calculate,
// and is some 30x faster than fnv_64_buf.
// 
// Not changing shader hash calculation as there are thousands of shaders already
// in the field, and there is no known bottleneck for that calculation.

static uint32_t crc32c_hw(uint32_t seed, const void *buffer, size_t length)
{
	try
	{
		const uint8_t *cast_buffer = static_cast<const uint8_t*>(buffer);

		return crc32c_append(seed, cast_buffer, length);
	}
	catch (...)
	{
		// Fatal error, but catch it and return null for hash.
		LogInfo("   ******* Exception caught while calculating crc32c_hw hash ******\n");
		return 0;
	}
}

// -----------------------------------------------------------------------------------------------

// Common routine to handle disassembling binary shaders to asm text.
// This is used whenever we need the Asm text.


// New version using Flugan's wrapper around D3DDisassemble to replace the
// problematic %f floating point values with %.9e, which is enough that a 32bit
// floating point value will be reproduced exactly:
static string BinaryToAsmText(const void *pShaderBytecode, size_t BytecodeLength,
		bool patch_cb_offsets,
		bool disassemble_undecipherable_data = true,
		int hexdump = 0, bool d3dcompiler_46_compat = false)
{
	string comments;
	vector<byte> byteCode(BytecodeLength);
	vector<byte> disassembly;
	HRESULT r;

	comments = "//   using 3Dmigoto v" + string(VER_FILE_VERSION_STR) + " on " + LogTime() + "//\n";
	memcpy(byteCode.data(), pShaderBytecode, BytecodeLength);

	r = disassembler(&byteCode, &disassembly, comments.c_str(), hexdump,
			d3dcompiler_46_compat, disassemble_undecipherable_data, patch_cb_offsets);
	if (FAILED(r)) {
		LogInfo("  disassembly failed. Error: %x\n", r);
		return "";
	}

	return string(disassembly.begin(), disassembly.end());
}

// Get the shader model from the binary shader bytecode.
//
// This used to disassemble, then search for the text string, but if we are going to
// do all that work, we might as well use the James-Jones decoder to get it.
// The other reason to do it this way is that we have seen multiple shader versions
// in Unity games, and the old technique of searching for the first uncommented line
// would fail.

// This is an interesting idea, but doesn't work well here because of project structure.
// for the moment, let's leave this here, but use the disassemble search approach.

//static string GetShaderModel(const void *pShaderBytecode)
//{
//	Shader *shader = DecodeDXBC((uint32_t*)pShaderBytecode);
//	if (shader == nullptr)
//		return "";
//
//	string shaderModel;
//	
//	switch (shader->eShaderType)
//	{
//	case PIXEL_SHADER:
//		shaderModel = "ps";
//		break;
//	case VERTEX_SHADER:
//		shaderModel = "vs";
//		break;
//	case GEOMETRY_SHADER:
//		shaderModel = "gs";
//		break;
//	case HULL_SHADER:
//		shaderModel = "hs";
//		break;
//	case DOMAIN_SHADER:
//		shaderModel = "ds";
//		break;
//	case COMPUTE_SHADER:
//		shaderModel = "cs";
//		break;
//	default:
//		return "";		// Failure.
//	}
//
//	shaderModel += "_" + shader->ui32MajorVersion;
//	shaderModel += "_" + shader->ui32MinorVersion;
//
//	delete shader;
//
//	return shaderModel;
//}

static string GetShaderModel(const void *pShaderBytecode, size_t bytecodeLength)
{
	string asmText = BinaryToAsmText(pShaderBytecode, bytecodeLength, false);
	if (asmText.empty())
		return "";

	// Read shader model. This is the first not commented line.
	char *pos = (char *)asmText.data();
	char *end = pos + asmText.size();
	while (pos[0] == '/' && pos < end)
	{
		while (pos[0] != 0x0a && pos < end) pos++;
		pos++;
	}
	// Extract model.
	char *eol = pos;
	while (eol[0] != 0x0a && pos < end) eol++;
	string shaderModel(pos, eol);

	return shaderModel;
}

// Create a text file containing text for the string specified.  Can be Asm or HLSL.
// If the file already exists and the caller did not specify overwrite (used
// for reassembled text), return that as an error to avoid overwriting previous
// work.

// We previously would overwrite the file only after checking if the contents were different,
// this relaxes that to just being same file name.

static HRESULT CreateTextFile(wchar_t *fullPath, string *asmText, bool overwrite)
{
	FILE *f;

	if (!overwrite) {
		_wfopen_s(&f, fullPath, L"rb");
		if (f)
		{
			fclose(f);
			LogInfoW(L"    CreateTextFile error: file already exists %s\n", fullPath);
			return ERROR_FILE_EXISTS;
		}
	}

	_wfopen_s(&f, fullPath, L"wb");
	if (f)
	{
		fwrite(asmText->data(), 1, asmText->size(), f);
		fclose(f);
	}

	return S_OK;
}

// Get shader type from asm, first non-commented line.  CS, PS, VS.
// Not sure this works on weird Unity variant with embedded types.


// Specific variant to name files consistently, so we know they are Asm text.

static HRESULT CreateAsmTextFile(wchar_t* fileDirectory, UINT64 hash, const wchar_t* shaderType, 
	const void *pShaderBytecode, size_t bytecodeLength, bool patch_cb_offsets)
{
	string asmText = BinaryToAsmText(pShaderBytecode, bytecodeLength, patch_cb_offsets);
	if (asmText.empty())
	{
		return E_OUTOFMEMORY;
	}

	wchar_t fullPath[MAX_PATH];
	swprintf_s(fullPath, MAX_PATH, L"%ls\\%016llx-%ls.txt", fileDirectory, hash, shaderType);

	HRESULT hr = CreateTextFile(fullPath, &asmText, false);

	if (SUCCEEDED(hr))
		LogInfoW(L"    storing disassembly to %s\n", fullPath);
	else
		LogInfoW(L"    error: %x, storing disassembly to %s\n", hr, fullPath);

	return hr;
}

// Specific variant to name files, so we know they are HLSL text.

static HRESULT CreateHLSLTextFile(UINT64 hash, string hlslText)
{

}

// -----------------------------------------------------------------------------------------------

// Parses the name of one of the IniParam constants: x, y, z, w, x1, y1, ..., z7, w7
static bool ParseIniParamName(const wchar_t *name, int *idx, float DirectX::XMFLOAT4::**component)
{
	int ret, len1, len2;
	wchar_t component_chr;
	size_t length = wcslen(name);

	ret = swscanf_s(name, L"%lc%n%u%n", &component_chr, 1, &len1, idx, &len2);

	// May or may not have matched index. Make sure entire string was
	// matched either way and check index is valid if it was matched:
	if (ret == 1 && len1 == length)
		*idx = 0;
	else if (ret != 2 || len2 != length)
		return false;

	switch (towlower(component_chr)) {
		case L'x':
			*component = &DirectX::XMFLOAT4::x;
			return true;
		case L'y':
			*component = &DirectX::XMFLOAT4::y;
			return true;
		case L'z':
			*component = &DirectX::XMFLOAT4::z;
			return true;
		case L'w':
			*component = &DirectX::XMFLOAT4::w;
			return true;
	}

	return false;
}

// -----------------------------------------------------------------------------------------------

BOOL CreateDirectoryEnsuringAccess(LPCWSTR path);
errno_t wfopen_ensuring_access(FILE** pFile, const wchar_t *filename, const wchar_t *mode);
void set_file_last_write_time(wchar_t *path, FILETIME *ftWrite, DWORD flags=0);
void touch_file(wchar_t *path, DWORD flags=0);
#define touch_dir(path) touch_file(path, FILE_FLAG_BACKUP_SEMANTICS)

bool check_interface_supported(IUnknown *unknown, REFIID riid);
void analyse_iunknown(IUnknown *unknown);

// For the time being, since we are not setup to use the Win10 SDK, we'll add
// these manually. Some games under Win10 are requesting these.

struct _declspec(uuid("9d06dffa-d1e5-4d07-83a8-1bb123f2f841")) ID3D11Device2;
struct _declspec(uuid("420d5b32-b90c-4da4-bef0-359f6a24a83a")) ID3D11DeviceContext2;
struct _declspec(uuid("A8BE2AC4-199F-4946-B331-79599FB98DE7")) IDXGISwapChain2;
struct _declspec(uuid("94D99BDB-F1F8-4AB0-B236-7DA0170EDAB1")) IDXGISwapChain3;
struct _declspec(uuid("3D585D5A-BD4A-489E-B1F4-3DBCB6452FFB")) IDXGISwapChain4;

std::string NameFromIID(IID id);

void WarnIfConflictingShaderExists(wchar_t *orig_path, const char *message = "");
static const char *end_user_conflicting_shader_msg =
	"Conflicting shaders present - please use uninstall.bat and reinstall the fix.\n";

struct OMState {
	UINT NumRTVs;
	ID3D11RenderTargetView *rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D11DepthStencilView *dsv;
	UINT UAVStartSlot;
	UINT NumUAVs;
	ID3D11UnorderedAccessView *uavs[D3D11_PS_CS_UAV_REGISTER_COUNT];
};

void save_om_state(ID3D11DeviceContext *context, struct OMState *state);
void restore_om_state(ID3D11DeviceContext *context, struct OMState *state);

extern IDXGISwapChain *last_fullscreen_swap_chain;
void install_crash_handler(int level);
