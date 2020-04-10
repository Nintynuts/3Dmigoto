#pragma once

#include "log.h"
#include "files.h"
#include "version.h"

#include <Windows.h>
#include <d3dcommon.h>
#include <string>
#include <vector>

// Custom #include handler used to track which shaders need to be reloaded after an included file is modified
class MigotoIncludeHandler : public ID3DInclude
{
	std::vector<std::string> dir_stack;

	void push_dir(const char* path);
public:
	MigotoIncludeHandler(const char* path);

	STDMETHOD(Open)(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);
	STDMETHOD(Close)(LPCVOID pData);
};

extern const char* end_user_conflicting_shader_msg;

static void WarnIfConflictingShaderExists(wchar_t *orig_path, const char* message = "");

// Helper routines for ReplaceShader, as a way to factor out some of the inline code, in
// order to make it more clear, and as a first step toward full refactoring.

// This routine exports the original binary shader from the game, the cso.  It is a hidden
// feature in the d3dx.ini.  Seems like it might be nice to have them named *_orig.bin, to
// make them more clear.
static void ExportOrigBinary(UINT64 hash, const wchar_t *pShaderType, const void* pShaderBytecode, SIZE_T pBytecodeLength);

static bool GetFileLastWriteTime(wchar_t *path, FILETIME* ftWrite);

static bool CheckCacheTimestamp(HANDLE binHandle, wchar_t *binPath, FILETIME& pTimeStamp);

static bool LoadCachedShader(wchar_t *binPath, const wchar_t *pShaderType,
	__out char*& pCode, SIZE_T& pCodeSize, std::string& pShaderModel, FILETIME& pTimeStamp);

// Load .bin shaders from the ShaderFixes folder as cached shaders.
// This will load either *_replace.bin, or *.bin variants.

static bool LoadBinaryShaders(__in UINT64 hash, const wchar_t *pShaderType,
	__out char*& pCode, SIZE_T& pCodeSize, std::string& pShaderModel, FILETIME& pTimeStamp);

// Load an HLSL text file as the replacement shader.  Recompile it using D3DCompile.
// If caching is enabled, save a .bin replacement for this new shader.

static bool ReplaceHLSLShader(__in UINT64 hash, const wchar_t *pShaderType,
	__in const void* pShaderBytecode, SIZE_T pBytecodeLength, const char* pOverrideShaderModel,
	__out char*& pCode, SIZE_T& pCodeSize, std::string& pShaderModel, FILETIME& pTimeStamp, std::wstring& pHeaderLine);

// Common routine to handle disassembling binary shaders to asm text.
// This is used whenever we need the Asm text.

// New version using Flugan's wrapper around D3DDisassemble to replace the
// problematic %f floating point values with %.9e, which is enough that a 32bit
// floating point value will be reproduced exactly:
static std::string BinaryToAsmText(const void* pShaderBytecode, size_t BytecodeLength,
	bool patch_cb_offsets,
	bool disassemble_undecipherable_data = true,
	int hexdump = 0, bool d3dcompiler_46_compat = true);

// Get the shader model from the binary shader bytecode.
//
// This used to disassemble, then search for the text std::string, but if we are going to
// do all that work, we might as well use the James-Jones decoder to get it.
// The other reason to do it this way is that we have seen multiple shader versions
// in Unity games, and the old technique of searching for the first uncommented line
// would fail.

// This is an interesting idea, but doesn't work well here because of project structure.
// for the moment, let's leave this here, but use the disassemble search approach.

//static std::string GetShaderModel(const void *pShaderBytecode)
//{
//	Shader *shader = DecodeDXBC((uint32_t*)pShaderBytecode);
//	if (shader == nullptr)
//		return "";
//
//	std::string shaderModel;
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
static std::string GetShaderModel(const void* pShaderBytecode, size_t bytecodeLength);

// Create a text file containing text for the std::string specified.  Can be Asm or HLSL.
// If the file already exists and the caller did not specify overwrite (used
// for reassembled text), return that as an error to avoid overwriting previous
// work.

// We previously would overwrite the file only after checking if the contents were different,
// this relaxes that to just being same file name.
static HRESULT CreateTextFile(wchar_t *fullPath, std::string* asmText, bool overwrite);

// Get shader type from asm, first non-commented line.  CS, PS, VS.
// Not sure this works on weird Unity variant with embedded types.

// Specific variant to name files consistently, so we know they are Asm text.
static HRESULT CreateAsmTextFile(wchar_t *fileDirectory, UINT64 hash, const wchar_t *shaderType,
	const void* pShaderBytecode, size_t bytecodeLength, bool patch_cb_offsets);