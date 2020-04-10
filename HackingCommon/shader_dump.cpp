#include "shader_dump.h"
#include "overlay.h"
#include "globals_common.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <d3dcompiler.h>

const char* end_user_conflicting_shader_msg = "Conflicting shaders present - please use uninstall.bat and reinstall the fix.\n";

void WarnIfConflictingFileExists(wchar_t *path, wchar_t *conflicting_path, const char* message)
{
	DWORD attrib = GetFileAttributes(conflicting_path);

	if (attrib == INVALID_FILE_ATTRIBUTES)
		return;

	LogOverlay(LOG_DIRE, "WARNING: %s\"%S\" conflicts with \"%S\"\n", message, conflicting_path, path);
}

void WarnIfConflictingShaderExists(wchar_t *orig_path, const char* message)
{
	wchar_t conflicting_path[MAX_PATH], * postfix;

	wcscpy_s(conflicting_path, MAX_PATH, orig_path);

	// If we're using a HLSL shader, make sure there are no conflicting
	// assembly shaders, either text or binary:
	postfix = wcsstr(conflicting_path, L"_replace");
	if (postfix) {
		wcscpy_s(postfix, conflicting_path + MAX_PATH - postfix, L".txt");
		WarnIfConflictingFileExists(orig_path, conflicting_path, message);
		wcscpy_s(postfix, conflicting_path + MAX_PATH - postfix, L".bin");
		WarnIfConflictingFileExists(orig_path, conflicting_path, message);
		return;
	}

	// If we're using an assembly shader, make sure there are no
	// conflicting HLSL shaders, either text or binary:
	postfix = wcsstr(conflicting_path, L".");
	if (postfix) {
		wcscpy_s(postfix, conflicting_path + MAX_PATH - postfix, L"_replace.txt");
		WarnIfConflictingFileExists(orig_path, conflicting_path, message);
		wcscpy_s(postfix, conflicting_path + MAX_PATH - postfix, L"_replace.bin");
		WarnIfConflictingFileExists(orig_path, conflicting_path, message);
		return;
	}
}

void ExportOrigBinary(UINT64 hash, const wchar_t *pShaderType, const void* pShaderBytecode, SIZE_T pBytecodeLength)
{
	wchar_t path[MAX_PATH];
	HANDLE f;
	bool exists = false;

	swprintf_s(path, MAX_PATH, L"%ls\\%016llx-%ls.bin", GB->SHADER_CACHE_PATH, hash, pShaderType);
	f = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f != INVALID_HANDLE_VALUE)
	{
		int cnt = 0;
		while (f != INVALID_HANDLE_VALUE)
		{
			// Check if same file.
			DWORD dataSize = GetFileSize(f, 0);
			char* buf = new char[dataSize];
			DWORD readSize;
			if (!ReadFile(f, buf, dataSize, &readSize, 0) || dataSize != readSize)
				LogInfo("  Error reading file.\n");
			CloseHandle(f);
			if (dataSize == pBytecodeLength && !memcmp(pShaderBytecode, buf, dataSize))
				exists = true;
			delete[] buf;
			if (exists)
				break;
			swprintf_s(path, MAX_PATH, L"%ls\\%016llx-%ls_%d.bin", GB->SHADER_CACHE_PATH, hash, pShaderType, ++cnt);
			f = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		}
	}
	if (!exists)
	{
		FILE* fw;
		wfopen_ensuring_access(&fw, path, L"wb");
		if (fw)
		{
			LogInfoW(L"    storing original binary shader to %s\n", path);
			fwrite(pShaderBytecode, 1, pBytecodeLength, fw);
			fclose(fw);
		}
		else
		{
			LogInfoW(L"    error storing original binary shader to %s\n", path);
		}
	}
}

bool GetFileLastWriteTime(wchar_t *path, FILETIME* ftWrite)
{
	HANDLE f;

	f = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == INVALID_HANDLE_VALUE)
		return false;

	GetFileTime(f, NULL, NULL, ftWrite);
	CloseHandle(f);
	return true;
}

bool CheckCacheTimestamp(HANDLE binHandle, wchar_t *binPath, FILETIME& pTimeStamp)
{
	FILETIME txtTime, binTime;
	wchar_t txtPath[MAX_PATH], * end = NULL;
	wcscpy_s(txtPath, MAX_PATH, binPath);
	end = wcsstr(txtPath, L".bin");
	wcscpy_s(end, sizeof(L".bin"), L".txt");
	if (GetFileLastWriteTime(txtPath, &txtTime) && GetFileTime(binHandle, NULL, NULL, &binTime)) {
		// We need to compare the timestamp on the .bin and .txt files.
		// This needs to be an exact match to ensure that the .bin file
		// corresponds to this .txt file (and we need to explicitly set
		// this timestamp when creating the .bin file). Just checking
		// for newer modification time is not enough, since the .txt
		// files in the zip files that fixes are distributed in contain
		// a timestamp that may be older than .bin files generated on
		// an end-users system.
		if (CompareFileTime(&binTime, &txtTime))
			return false;

		// It no longer matters which timestamp we save for later
		// comparison, since they need to match, but we save the .txt
		// file's timestamp since that is the one we are comparing
		// against later.
		pTimeStamp = txtTime;
		return true;
	}

	// If we couldn't get the timestamps it probably means the
	// corresponding .txt file no longer exists. This is actually a bit of
	// an odd (but not impossible) situation to be in - if a user used
	// uninstall.bat when updating a fix they should have removed any stale
	// .bin files as well, and if they didn't use uninstall.bat then they
	// should only be adding new files... so how did a shader that used to
	// be present disappear but leave the cache next to it?
	//
	// A shaderhacker might hit this if they removed the .txt file but not
	// the .bin file, but we could consider that to be user error, so it's
	// not clear any policy here would be correct. Alternatively, a fix
	// might have been shipped with only .bin files - historically we have
	// allowed (but discouraged) that scenario, so for now we issue a
	// warning but allow it.
	LogInfo("    WARNING: Unable to validate timestamp of %S"
		" - no corresponding .txt file?\n", binPath);
	return true;
}

bool LoadCachedShader(wchar_t *binPath, const wchar_t *pShaderType,
	__out char*& pCode, SIZE_T& pCodeSize, std::string& pShaderModel, FILETIME& pTimeStamp)
{
	HANDLE f;
	DWORD codeSize, readSize;

	f = CreateFile(binPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == INVALID_HANDLE_VALUE)
		return false;

	if (!CheckCacheTimestamp(f, binPath, pTimeStamp)) {
		LogInfoW(L"    Discarding stale cached shader: %s\n", binPath);
		goto bail_close_handle;
	}

	LogInfoW(L"    Replacement binary shader found: %s\n", binPath);
	WarnIfConflictingShaderExists(binPath, end_user_conflicting_shader_msg);

	codeSize = GetFileSize(f, 0);
	pCode = new char[codeSize];
	if (!ReadFile(f, pCode, codeSize, &readSize, 0)
		|| codeSize != readSize)
	{
		LogInfo("    Error reading binary file.\n");
		goto err_free_code;
	}

	pCodeSize = codeSize;
	LogInfo("    Bytecode loaded. Size = %Iu\n", pCodeSize);
	CloseHandle(f);

	pShaderModel = "bin";		// tag it as reload candidate, but needing disassemble

	return true;

err_free_code:
	delete[] pCode;
	pCode = NULL;
bail_close_handle:
	CloseHandle(f);
	return false;
}

// Load .bin shaders from the ShaderFixes folder as cached shaders.
// This will load either *_replace.bin, or *.bin variants.

bool LoadBinaryShaders(__in UINT64 hash, const wchar_t *pShaderType,
	__out char*& pCode, SIZE_T& pCodeSize, std::string& pShaderModel, FILETIME& pTimeStamp)
{
	wchar_t path[MAX_PATH];

	swprintf_s(path, MAX_PATH, L"%ls\\%016llx-%ls_replace.bin", GB->SHADER_PATH, hash, pShaderType);
	if (LoadCachedShader(path, pShaderType, pCode, pCodeSize, pShaderModel, pTimeStamp))
		return true;

	// If we can't find an HLSL compiled version, look for ASM assembled one.
	swprintf_s(path, MAX_PATH, L"%ls\\%016llx-%ls.bin", GB->SHADER_PATH, hash, pShaderType);
	return LoadCachedShader(path, pShaderType, pCode, pCodeSize, pShaderModel, pTimeStamp);
}

// Load an HLSL text file as the replacement shader.  Recompile it using D3DCompile.
// If caching is enabled, save a .bin replacement for this new shader.

bool ReplaceHLSLShader(__in UINT64 hash, const wchar_t *pShaderType,
	__in const void* pShaderBytecode, SIZE_T pBytecodeLength, const char* pOverrideShaderModel,
	__out char*& pCode, SIZE_T& pCodeSize, std::string& pShaderModel, FILETIME& pTimeStamp, std::wstring& pHeaderLine)
{
	wchar_t path[MAX_PATH];
	HANDLE f;
	std::string shaderModel;

	swprintf_s(path, MAX_PATH, L"%ls\\%016llx-%ls_replace.txt", GB->SHADER_PATH, hash, pShaderType);
	f = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f != INVALID_HANDLE_VALUE)
	{
		LogInfo("    Replacement shader found. Loading replacement HLSL code.\n");
		WarnIfConflictingShaderExists(path, end_user_conflicting_shader_msg);

		DWORD srcDataSize = GetFileSize(f, 0);
		char* srcData = new char[srcDataSize];
		DWORD readSize;
		FILETIME ftWrite;
		if (!ReadFile(f, srcData, srcDataSize, &readSize, 0)
			|| !GetFileTime(f, NULL, NULL, &ftWrite)
			|| srcDataSize != readSize)
			LogInfo("    Error reading file.\n");
		CloseHandle(f);
		LogInfo("    Source code loaded. Size = %d\n", srcDataSize);

		// Disassemble old shader to get shader model.
		shaderModel = GetShaderModel(pShaderBytecode, pBytecodeLength);
		if (shaderModel.empty())
		{
			LogInfo("    disassembly of original shader failed.\n");

			delete[] srcData;
		}
		else
		{
			// Any HLSL compiled shaders are reloading candidates, if moved to ShaderFixes
			pShaderModel = shaderModel;
			pTimeStamp = ftWrite;
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf8_to_utf16;
			pHeaderLine = utf8_to_utf16.from_bytes(srcData, strchr(srcData, '\n'));

			// Way too many obscure interractions in this function, using another
			// temporary variable to not modify anything already here and reduce
			// the risk of breaking it in some subtle way:
			const char* tmpShaderModel;
			char apath[MAX_PATH];

			if (pOverrideShaderModel)
				tmpShaderModel = pOverrideShaderModel;
			else
				tmpShaderModel = shaderModel.c_str();

			// Compile replacement.
			LogInfo("    compiling replacement HLSL code with shader model %s\n", tmpShaderModel);

			// TODO: Add #defines for StereoParams and IniParams

			ID3DBlob* errorMsgs; // FIXME: This can leak
			ID3DBlob* compiledOutput = 0;
			// Pass the real filename and use the standard include handler so that
			// #include will work with a relative path from the shader itself.
			// Later we could add a custom include handler to track dependencies so
			// that we can make reloading work better when using includes:
			wcstombs(apath, path, MAX_PATH);
			MigotoIncludeHandler include_handler(apath);
			HRESULT ret = D3DCompile(srcData, srcDataSize, apath, 0,
				GB->recursive_include == -1 ? D3D_COMPILE_STANDARD_FILE_INCLUDE : &include_handler,
				"main", tmpShaderModel, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &compiledOutput, &errorMsgs);
			delete[] srcData; srcData = 0;
			if (compiledOutput)
			{
				pCodeSize = compiledOutput->GetBufferSize();
				pCode = new char[pCodeSize];
				memcpy(pCode, compiledOutput->GetBufferPointer(), pCodeSize);
				compiledOutput->Release(); compiledOutput = 0;
			}

			LogInfo("    compile result of replacement HLSL shader: %x\n", ret);

			if (LogFile && errorMsgs)
			{
				LPVOID errMsg = errorMsgs->GetBufferPointer();
				SIZE_T errSize = errorMsgs->GetBufferSize();
				LogInfo("--------------------------------------------- BEGIN ---------------------------------------------\n");
				fwrite(errMsg, 1, errSize - 1, LogFile);
				LogInfo("---------------------------------------------- END ----------------------------------------------\n");
				errorMsgs->Release();
			}

			// Cache binary replacement.
			if (GB->CACHE_SHADERS && pCode)
			{
				swprintf_s(path, MAX_PATH, L"%ls\\%016llx-%ls_replace.bin", GB->SHADER_PATH, hash, pShaderType);
				FILE* fw;
				wfopen_ensuring_access(&fw, path, L"wb");
				if (fw)
				{
					LogInfo("    storing compiled shader to %S\n", path);
					fwrite(pCode, 1, pCodeSize, fw);
					fclose(fw);

					// Set the last modified timestamp on the cached shader to match the
					// .txt file it is created from, so we can later check its validity:
					set_file_last_write_time(path, &ftWrite);
				}
				else
					LogInfo("    error writing compiled shader to %S\n", path);
			}
		}
	}
	return !!pCode;
}


// Common routine to handle disassembling binary shaders to asm text.
// This is used whenever we need the Asm text.

// New version using Flugan's wrapper around D3DDisassemble to replace the
// problematic %f floating point values with %.9e, which is enough that a 32bit
// floating point value will be reproduced exactly:
std::string BinaryToAsmText(const void* pShaderBytecode, size_t BytecodeLength,
	bool patch_cb_offsets,
	bool disassemble_undecipherable_data,
	int hexdump, bool d3dcompiler_46_compat)
{
	std::string comments;
	std::vector<byte> byteCode(BytecodeLength);
	std::vector<byte> disassembly;
	HRESULT r;

	comments = "//   using 3Dmigoto v" + std::string(VER_FILE_VERSION_STR) + " on " + LogTime() + "//\n";
	memcpy(byteCode.data(), pShaderBytecode, BytecodeLength);

#if MIGOTO_DX == 9
	r = disassemblerDX9(&byteCode, &disassembly, comments.c_str());
#elif MIGOTO_DX == 11
	r = disassembler(&byteCode, &disassembly, comments.c_str(), hexdump,
		d3dcompiler_46_compat, disassemble_undecipherable_data, patch_cb_offsets);
#endif // MIGOTO_DX
	if (FAILED(r)) {
		LogInfo("  disassembly failed. Error: %x\n", r);
		return "";
	}

	return std::string(disassembly.begin(), disassembly.end());
}

// Get the shader model from the binary shader bytecode.
//
// This used to disassemble, then search for the text std::string, but if we are going to
// do all that work, we might as well use the James-Jones decoder to get it.
// The other reason to do it this way is that we have seen multiple shader versions
// in Unity games, and the old technique of searching for the first uncommented line
// would fail.

// This is an interesting idea, but doesn't work well here because of project structure.
// for the moment, let's leave this here, but use the disassemble search approach.

//std::string GetShaderModel(const void *pShaderBytecode)
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
std::string GetShaderModel(const void* pShaderBytecode, size_t bytecodeLength)
{
	std::string asmText = BinaryToAsmText(pShaderBytecode, bytecodeLength, false);
	if (asmText.empty())
		return "";

	// Read shader model. This is the first not commented line.
	char* pos = (char*)asmText.data();
	char* end = pos + asmText.size();
	while ((pos[0] == '/' || pos[0] == '\n') && pos < end)
	{
		while (pos[0] != 0x0a && pos < end) pos++;
		pos++;
	}
	// Extract model.
	char* eol = pos;
	while (eol[0] != 0x0a && pos < end) eol++;
	std::string shaderModel(pos, eol);

	return shaderModel;
}

// Create a text file containing text for the std::string specified.  Can be Asm or HLSL.
// If the file already exists and the caller did not specify overwrite (used
// for reassembled text), return that as an error to avoid overwriting previous
// work.

// We previously would overwrite the file only after checking if the contents were different,
// this relaxes that to just being same file name.
HRESULT CreateTextFile(wchar_t *fullPath, std::string* asmText, bool overwrite)
{
	FILE* f;

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
HRESULT CreateAsmTextFile(wchar_t *fileDirectory, UINT64 hash, const wchar_t *shaderType,
	const void* pShaderBytecode, size_t bytecodeLength, bool patch_cb_offsets)
{
	std::string asmText = BinaryToAsmText(pShaderBytecode, bytecodeLength, patch_cb_offsets);
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