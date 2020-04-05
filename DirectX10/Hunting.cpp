#include <string>

#include "HackerDevice.h"
#include "input.h"
#include "Override.h"
#include "globals.h"
#include "DecompileHLSL.h"

#include <util.h>
#include <texture.h>

static bool gReloadConfigPending = false;


static int StrRenderTarget2D(char *buf, size_t size, D3D10_TEXTURE2D_DESC *desc)
{
	return _snprintf_s(buf, size, size, "type=Texture2D Width=%u Height=%u MipLevels=%u "
		"ArraySize=%u RawFormat=%u Format=\"%s\" SampleDesc.Count=%u "
		"SampleDesc.Quality=%u Usage=%u BindFlags=%u "
		"CPUAccessFlags=%u MiscFlags=%u",
		desc->Width, desc->Height, desc->MipLevels,
		desc->ArraySize, desc->Format,
		TexFormatStr(desc->Format), desc->SampleDesc.Count,
		desc->SampleDesc.Quality, desc->Usage, desc->BindFlags,
		desc->CPUAccessFlags, desc->MiscFlags);
}

static int StrRenderTarget3D(char *buf, size_t size, D3D10_TEXTURE3D_DESC *desc)
{

	return _snprintf_s(buf, size, size, "type=Texture3D Width=%u Height=%u Depth=%u "
		"MipLevels=%u RawFormat=%u Format=\"%s\" Usage=%u BindFlags=%u "
		"CPUAccessFlags=%u MiscFlags=%u",
		desc->Width, desc->Height, desc->Depth,
		desc->MipLevels, desc->Format,
		TexFormatStr(desc->Format), desc->Usage, desc->BindFlags,
		desc->CPUAccessFlags, desc->MiscFlags);
}

static int StrRenderTarget(char *buf, size_t size, struct ResourceInfo &info)
{
	switch (info.type) {
		case D3D10_RESOURCE_DIMENSION_TEXTURE2D:
			return StrRenderTarget2D(buf, size, &info.tex2d_desc);
		case D3D10_RESOURCE_DIMENSION_TEXTURE3D:
			return StrRenderTarget3D(buf, size, &info.tex3d_desc);
		default:
			return _snprintf_s(buf, size, size, "type=%i\n", info.type);
	}
}

// bo3b: For this routine, we have a lot of warnings in x64, from converting a size_t result into the needed
//  DWORD type for the Write calls.  These are writing 256 byte strings, so there is never a chance that it 
//  will lose data, so rather than do anything heroic here, I'm just doing type casts on the strlen function.

DWORD castStrLen(const char* string)
{
	return (DWORD)strlen(string);
}

static void DumpUsage()
{
	wchar_t dir[MAX_PATH];
	GetModuleFileName(0, dir, MAX_PATH);
	wcsrchr(dir, L'\\')[1] = 0;
	wcscat(dir, L"ShaderUsage.txt");
	HANDLE f = CreateFile(dir, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f != INVALID_HANDLE_VALUE)
	{
		if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
		char buf[256];
		DWORD written;
		for (auto i = G->mVertexShaders.mInfo.begin(); i != G->mVertexShaders.mInfo.end(); ++i)
		{
			sprintf(buf, "<VertexShader hash=\"%016llx\">\n  <CalledPixelShaders>", i->first);
			WriteFile(f, buf, castStrLen(buf), &written, 0);
			for (auto j = i->second.PartnerShaders.begin(); j != i->second.PartnerShaders.end(); ++j)
			{
				sprintf(buf, "%016llx ", *j);
				WriteFile(f, buf, castStrLen(buf), &written, 0);
			}
			const char *REG_HEADER = "</CalledPixelShaders>\n";
			WriteFile(f, REG_HEADER, castStrLen(REG_HEADER), &written, 0);
			for (auto k = i->second.ResourceRegisters.begin(); k != i->second.ResourceRegisters.end(); ++k)
			{
				for each (auto l in k->second)
				{
					if (l.handle)
					{
						UINT64 id = G->mRenderTargets.mRegistered[l.handle];
						sprintf(buf, "  <Register id=%d handle=%p>%016llx</Register>\n", l.orig_hash, l.handle, id);
						WriteFile(f, buf, castStrLen(buf), &written, 0);
					}
				}
			}
			const char *FOOTER = "</VertexShader>\n";
			WriteFile(f, FOOTER, castStrLen(FOOTER), &written, 0);
		}
		for (auto i = G->mPixelShaders.mInfo.begin(); i != G->mPixelShaders.mInfo.end(); ++i)
		{
			sprintf(buf, "<PixelShader hash=\"%016llx\">\n"
				"  <ParentVertexShaders>", i->first);
			WriteFile(f, buf, castStrLen(buf), &written, 0);
			for (auto j = i->second.PartnerShaders.begin(); j != i->second.PartnerShaders.end(); ++j)
			{
				sprintf(buf, "%016llx ", *j);
				WriteFile(f, buf, castStrLen(buf), &written, 0);
			}
			const char *REG_HEADER = "</ParentVertexShaders>\n";
			WriteFile(f, REG_HEADER, castStrLen(REG_HEADER), &written, 0);
			for (auto k = i->second.ResourceRegisters.begin(); k != i->second.ResourceRegisters.end(); ++k)
			{
				for each (auto l in k->second)
				{
					if (l.handle)
					{
						UINT64 id = G->mRenderTargets.mRegistered[l.handle];
						sprintf(buf, "  <Register id=%d handle=%p>%016llx</Register>\n", l.orig_hash, l.handle, id);
						WriteFile(f, buf, castStrLen(buf), &written, 0);
					}
				}
			}
			int pos = 0;
			for (auto m = i->second.RenderTargets.begin(); m != i->second.RenderTargets.end(); m++, pos++) {
				for (auto o = (*m).begin(); o != (*m).end(); o++) {
					UINT64 id = G->mRenderTargets.mRegistered[o->handle];
					sprintf(buf, "  <RenderTarget id=%d handle=%p>%016llx</RenderTarget>\n", pos, *o, id);
					WriteFile(f, buf, castStrLen(buf), &written, 0);
				}
			}
			for (auto n = i->second.DepthTargets.begin(); n != i->second.DepthTargets.end(); n++) {
				UINT64 id = G->mRenderTargets.mRegistered[n->handle];
				sprintf(buf, "  <DepthTarget handle=%p>%016llx</DepthTarget>\n", *n, id);
				WriteFile(f, buf, castStrLen(buf), &written, 0);
			}
			const char *FOOTER = "</PixelShader>\n";
			WriteFile(f, FOOTER, castStrLen(FOOTER), &written, 0);
		}
		for (auto j = G->mRenderTargets.mInfo.begin(); j != G->mRenderTargets.mInfo.end(); j++) {
			_snprintf_s(buf, 256, 256, "<RenderTarget hash=%016llx ", j->first);
			WriteFile(f, buf, castStrLen(buf), &written, 0);
			StrRenderTarget(buf, 256, j->second);
			WriteFile(f, buf, castStrLen(buf), &written, 0);
			const char *FOOTER = "></RenderTarget>\n";
			WriteFile(f, FOOTER, castStrLen(FOOTER), &written, 0);
		}
		for (auto j = G->mDepthTargetInfo.begin(); j != G->mDepthTargetInfo.end(); j++) {
			_snprintf_s(buf, 256, 256, "<DepthTarget hash=%016llx ", j->first);
			WriteFile(f, buf, castStrLen(buf), &written, 0);
			StrRenderTarget(buf, 256, j->second);
			WriteFile(f, buf, castStrLen(buf), &written, 0);
			const char *FOOTER = "></DepthTarget>\n";
			WriteFile(f, FOOTER, castStrLen(FOOTER), &written, 0);
		}
		if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
		CloseHandle(f);
	}
	else
	{
		LogInfo("Error dumping ShaderUsage.txt\n");

	}
}

// Convenience class to avoid passing wrong objects all of Blob type.
// For strong type checking.  Already had a couple of bugs with generic ID3DBlobs.

class AsmTextBlob : public ID3DBlob
{
};

// Get the text disassembly of the shader byte code specified.

static AsmTextBlob* GetDisassembly(ID3DBlob* pCode)
{
	ID3DBlob *disassembly;

	HRESULT ret = D3DDisassemble(pCode->GetBufferPointer(), pCode->GetBufferSize(), D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS, 0,
		&disassembly);
	if (FAILED(ret))
	{
		LogInfo("    disassembly of original shader failed:\n");
		return NULL;
	}

	return (AsmTextBlob*)disassembly;
}

// Write the decompiled text as HLSL source code to the txt file.
// Now also writing the ASM text to the bottom of the file, commented out.
// This keeps the ASM with the HLSL for reference and should be more convenient.
//
// This will not overwrite any file that is already there. 
// The assumption is that the shaderByteCode that we have here is always the most up to date,
// and thus is not different than the file on disk.
// If a file was already extant in the ShaderFixes, it will be picked up at game launch as the master shaderByteCode.

static bool WriteHLSL(string hlslText, AsmTextBlob* asmTextBlob, UINT64 hash, wstring shaderType)
{
	wchar_t fullName[MAX_PATH];
	FILE *fw;

	wsprintf(fullName, L"%ls\\%08lx%08lx-%ls_replace.txt", G->SHADER_PATH, (UINT32)(hash >> 32), (UINT32)hash, shaderType.c_str());
	_wfopen_s(&fw, fullName, L"rb");
	if (fw)
	{
		LogInfoW(L"    marked shader file already exists: %s\n", fullName);
		fclose(fw);
		_wfopen_s(&fw, fullName, L"ab");
		if (fw) {
			fprintf_s(fw, " ");					// Touch file to update mod date as a convenience.
			fclose(fw);
		}
		BeepShort();						// Short High beep for for double beep that it's already there.
		return true;
	}

	_wfopen_s(&fw, fullName, L"wb");
	if (!fw)
	{
		LogInfoW(L"    error storing marked shader to %s\n", fullName);
		return false;
	}

	LogInfoW(L"    storing patched shader to %s\n", fullName);

	fwrite(hlslText.c_str(), 1, hlslText.size(), fw);

	fprintf_s(fw, "\n\n/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	// Size - 1 to strip NULL terminator
	fwrite(asmTextBlob->GetBufferPointer(), 1, asmTextBlob->GetBufferSize() - 1, fw);
	fprintf_s(fw, "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/\n");

	fclose(fw);
	return true;
}


// Decompile code passed in as assembly text, and shader byte code.
// This is pretty heavyweight obviously, so it is only being done during Mark operations.
// Todo: another copy/paste job, we really need some subroutines, utility library.

static string Decompile(ID3DBlob* pShaderByteCode, AsmTextBlob* disassembly)
{
	LogInfo("    creating HLSL representation.\n");

	bool patched = false;
	string shaderModel;
	bool errorOccurred = false;
	ParseParameters p;
	p.bytecode = pShaderByteCode->GetBufferPointer();
	p.decompiled = (const char *)disassembly->GetBufferPointer();
	p.decompiledSize = disassembly->GetBufferSize();
	p.G = &G->decompiler_settings;
	p.ZeroOutput = false;
	const string decompiledCode = DecompileBinaryHLSL(p, patched, shaderModel, errorOccurred);

	if (!decompiledCode.size())
	{
		LogInfo("    error while decompiling.\n");
	}

	return decompiledCode;
}


// Write the disassembly to the text file.
// If the file already exists, return an error, to avoid overwrite.  
// Generally if the file is already there, the code we would write on Mark is the same anyway.

static bool WriteDisassembly(UINT64 hash, wstring shaderType, AsmTextBlob* asmTextBlob)
{
	wchar_t fullName[MAX_PATH];
	FILE *f;

	wsprintf(fullName, L"%ls\\%08lx%08lx-%ls.txt", G->SHADER_PATH, (UINT32)(hash >> 32), (UINT32)(hash), shaderType.c_str());

	// Check if the file already exists.
	_wfopen_s(&f, fullName, L"rb");
	if (f)
	{
		LogInfoW(L"    Shader Mark .bin file already exists: %s\n", fullName);
		fclose(f);
		return false;
	}

	_wfopen_s(&f, fullName, L"wb");
	if (!f)
	{
		LogInfoW(L"    Shader Mark could not write asm text file: %s\n", fullName);
		return false;
	}

	// Size - 1 to strip NULL terminator
	fwrite(asmTextBlob->GetBufferPointer(), 1, asmTextBlob->GetBufferSize() - 1, f);
	fclose(f);
	LogInfoW(L"    storing disassembly to %s\n", fullName);

	return true;
}

// Different version that takes asm text already.

static string GetShaderModel(AsmTextBlob* asmTextBlob)
{
	// Read shader model. This is the first not commented line.
	char *pos = (char *)asmTextBlob->GetBufferPointer();
	char *end = pos + asmTextBlob->GetBufferSize();
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


// Compile a new shader from  HLSL text input, and report on errors if any.
// Return the binary blob of pCode to be activated with CreateVertexShader or CreatePixelShader.
// If the timeStamp has not changed from when it was loaded, skip the recompile, and return false as not an 
// error, but skipped.  On actual errors, return true so that we bail out.

static bool CompileShader(wchar_t *shaderFixPath, wchar_t *fileName, const char *shaderModel, UINT64 hash, wstring shaderType, FILETIME* timeStamp,
	_Outptr_ ID3DBlob** pCode)
{
	*pCode = nullptr;
	wchar_t fullName[MAX_PATH];
	wsprintf(fullName, L"%s\\%s", shaderFixPath, fileName);

	HANDLE f = CreateFile(fullName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == INVALID_HANDLE_VALUE)
	{
		LogInfo("    ReloadShader shader not found: %ls\n", fullName);

		return true;
	}


	DWORD srcDataSize = GetFileSize(f, 0);
	char *srcData = new char[srcDataSize];
	DWORD readSize;
	FILETIME curFileTime;

	if (!ReadFile(f, srcData, srcDataSize, &readSize, 0)
		|| !GetFileTime(f, NULL, NULL, &curFileTime)
		|| srcDataSize != readSize)
	{
		LogInfo("    Error reading txt file.\n");

		return true;
	}
	CloseHandle(f);

	// Check file time stamp, and only recompile shaders that have been edited since they were loaded.
	// This dramatically improves the F10 reload speed.
	if (!CompareFileTime(timeStamp, &curFileTime))
	{
		return false;
	}
	*timeStamp = curFileTime;

	LogInfo("   >Replacement shader found. Re-Loading replacement HLSL code from %ls\n", fileName);
	LogInfo("    Reload source code loaded. Size = %d\n", srcDataSize);
	LogInfo("    compiling replacement HLSL code with shader model %s\n", shaderModel);


	ID3DBlob* pByteCode = nullptr;
	ID3DBlob* pErrorMsgs = nullptr;
	HRESULT ret = D3DCompile(srcData, srcDataSize, "wrapper1349", 0, ((ID3DInclude*)(UINT_PTR)1),
		"main", shaderModel, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &pByteCode, &pErrorMsgs);

	delete srcData; srcData = 0;

	// bo3b: pretty sure that we do not need to copy the data. That data is what we need to pass to CreateVertexShader
	// Example taken from: http://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	//char *pCode = 0;
	//SIZE_T pCodeSize;
	//if (pCompiledOutput)
	//{
	//	pCodeSize = pCompiledOutput->GetBufferSize();
	//	pCode = new char[pCodeSize];
	//	memcpy(pCode, pCompiledOutput->GetBufferPointer(), pCodeSize);
	//	pCompiledOutput->Release(); pCompiledOutput = 0;
	//}

	LogInfo("    compile result of replacement HLSL shader: %x\n", ret);

	if (LogFile && pErrorMsgs)
	{
		LPVOID errMsg = pErrorMsgs->GetBufferPointer();
		SIZE_T errSize = pErrorMsgs->GetBufferSize();
		LogInfo("--------------------------------------------- BEGIN ---------------------------------------------\n");
		fwrite(errMsg, 1, errSize - 1, LogFile);
		LogInfo("---------------------------------------------- END ----------------------------------------------\n");
		pErrorMsgs->Release();
	}

	if (FAILED(ret))
	{
		if (pByteCode)
		{
			pByteCode->Release();
			pByteCode = 0;
		}
		return true;
	}


	// Write replacement .bin if necessary
	if (G->CACHE_SHADERS && pByteCode)
	{
		wchar_t val[MAX_PATH];
		wsprintf(val, L"%ls\\%08lx%08lx-%ls_replace.bin", shaderFixPath, (UINT32)(hash >> 32), (UINT32)(hash), shaderType.c_str());
		FILE *fw;
		_wfopen_s(&fw, val, L"wb");
		if (LogFile)
		{
			char fileName[MAX_PATH];
			wcstombs(fileName, val, MAX_PATH);
			if (fw)
				LogInfo("    storing compiled shader to %s\n", fileName);
			else
				LogInfo("    error writing compiled shader to %s\n", fileName);
		}
		if (fw)
		{
			fwrite(pByteCode->GetBufferPointer(), 1, pByteCode->GetBufferSize(), fw);
			fclose(fw);
		}
	}

	// pCode on return == NULL for error cases, valid if made it this far.
	*pCode = pByteCode;

	return true;
}

// Strategy: When the user hits F10 as the reload key, we want to reload all of the hand-patched shaders in
//	the ShaderFixes folder, and make them live in game.  That will allow the user to test out fixes on the 
//	HLSL .txt files and do live experiments with fixes.  This makes it easier to figure things out.
//	To do that, we need to patch what is being sent to VSSetShader and PSSetShader, as they get activated
//	in game.  Since the original shader is already on the GPU from CreateVertexShader and CreatePixelShader,
//	we need to override it on the fly. We cannot change what the game originally sent as the shader request,
//	nor their storage location, because we cannot guarantee that they didn't make a copy and use it. So, the
//	item to go on is the ID3D10VertexShader* that the game received back from CreateVertexShader and will thus
//	use to activate it with VSSetShader.
//	To do the override, we've made a new Map that maps the original ID3D10VertexShader* to the new one, and
//	in VSSetShader, we will look for a map match, and if we find it, we'll apply the override of the newly
//	loaded shader.
//	Here in ReloadShader, we need to set up to make that possible, by filling in the mReloadedVertexShaders
//	map with <old,new>. In this spot, we have been notified by the user via F10 or whatever input that they
//	want the shaders reloaded. We need to take each shader hlsl.txt file, recompile it, call CreateVertexShader
//	on it to make it available in the GPU, and save the new ID3D10VertexShader* in our <old,new> map. We get the
//	old ID3D10VertexShader* reference by looking that up in the complete mVertexShaders map, by using the hash
//	number we can get from the file name itself.
//	Notably, if the user does multiple iterations, we'll still only use the same number of overrides, because
//	the map will replace the last one. This probably does leak vertex shaders on the GPU though.

// Todo: this is not a particularly good spot for the routine.  Need to move these compile/dissassemble routines
//	including those in Direct3D11Device.h into a separate file and include a .h file.
//	This routine plagarized from ReplaceShaders.

// Reload all of the patched shaders from the ShaderFixes folder, and add them to the override map, so that the
// new version will be used at VSSetShader and PSSetShader.
// File names are uniform in the form: 3c69e169edc8cd5f-ps_replace.txt

static bool ReloadShader(wchar_t *shaderPath, wchar_t *fileName, ID3D10Device *realDevice)
{
	UINT64 hash;
	ID3D10DeviceChild* oldShader = NULL;
	ID3D10DeviceChild* replacement = NULL;
	//ID3D10ClassLinkage* classLinkage;
	ID3DBlob* shaderCode;
	string shaderModel;
	wstring shaderType;		// "vs" or "ps" maybe "gs"
	FILETIME timeStamp;
	HRESULT hr = E_FAIL;

	// Extract hash from first 16 characters of file name so we can look up details by hash
	wstring ws = fileName;
	hash = stoull(ws.substr(0, 16), NULL, 16);

	// Find the original shader bytecode in the mReloadedShaders Map. This map contains entries for all
	// shaders from the ShaderFixes and ShaderCache folder, and can also include .bin files that were loaded directly.
	// We include ShaderCache because that allows moving files into ShaderFixes as they are identified.
	// This needs to use the value to find the key, so a linear search.
	// It's notable that the map can contain multiple copies of the same hash, used for different visual
	// items, but with same original code.  We need to update all copies.
	for each (pair<ID3D10DeviceChild *, OriginalShaderInfo> iter in G->mReloadedShaders)
	{
		if (iter.second.hash == hash)
		{
			oldShader = iter.first;
			//classLinkage = iter.second.linkage;
			shaderModel = iter.second.shaderModel;
			shaderType = iter.second.shaderType;
			timeStamp = iter.second.timeStamp;
			shaderCode = iter.second.byteCode;

			// If we didn't find an original shader, that is OK, because it might not have been loaded yet.
			// Just skip it in that case, because the new version will be loaded when it is used.
			if (oldShader == NULL)
			{
				LogInfo("> failed to find original shader in mReloadedShaders: %ls\n", fileName);
				continue;
			}

			// If shaderModel is "bin", that means the original was loaded as a binary object, and thus shaderModel is unknown.
			// Disassemble the binary to get that string.
			if (shaderModel.compare("bin") == 0)
			{
				AsmTextBlob* asmTextBlob = GetDisassembly(shaderCode);
				if (!asmTextBlob)
					return false;
				shaderModel = GetShaderModel(asmTextBlob);
				if (shaderModel.empty())
					return false;
				G->mReloadedShaders[oldShader].shaderModel = shaderModel;
			}

			// Compile anew. If timestamp is unchanged, the code is unchanged, continue to next shader.
			ID3DBlob *pShaderBytecode = NULL;
			if (!CompileShader(shaderPath, fileName, shaderModel.c_str(), hash, shaderType, &timeStamp, &pShaderBytecode))
				continue;

			// If we compiled but got nothing, that's a fatal error we need to report.
			if (pShaderBytecode == NULL)
				return false;

			// Update timestamp, since we have an edited file.
			G->mReloadedShaders[oldShader].timeStamp = timeStamp;

			// This needs to call the real CreateVertexShader, not our wrapped version
			if (shaderType.compare(L"vs") == 0)
			{
				hr = realDevice->CreateVertexShader(pShaderBytecode->GetBufferPointer(), pShaderBytecode->GetBufferSize(),
					(ID3D10VertexShader**) &replacement);
			}
			else if (shaderType.compare(L"ps") == 0)
			{
				hr = realDevice->CreatePixelShader(pShaderBytecode->GetBufferPointer(), pShaderBytecode->GetBufferSize(),
					(ID3D10PixelShader**) &replacement);
			}
			if (FAILED(hr))
				return false;


			// If we have an older reloaded shader, let's release it to avoid a memory leak.  This only happens after 1st reload.
			// New shader is loaded on GPU and ready to be used as override in VSSetShader or PSSetShader
			if (G->mReloadedShaders[oldShader].replacement != NULL)
				G->mReloadedShaders[oldShader].replacement->Release();
			G->mReloadedShaders[oldShader].replacement = replacement;

			// New binary shader code, to replace the prior loaded shader byte code. 
			shaderCode->Release();
			G->mReloadedShaders[oldShader].byteCode = pShaderBytecode;

			LogInfo("> successfully reloaded shader: %ls\n", fileName);
		}
	}	// for every registered shader in mReloadedShaders 

	return true;
}

// When a shader is marked by the user, we want to automatically move it to the ShaderFixes folder
// The universal way to do this is to keep the shaderByteCode around, and when mark happens, use that as
// the replacement and build code to match.  This handles all the variants of preload, cache, hlsl 
// or not, and allows creating new files on a first run.  Should be handy.

static void CopyToFixes(UINT64 hash, ID3D10Device *device)
{
	bool success = false;
	string shaderModel;
	AsmTextBlob* asmTextBlob;
	string decompiled;

	// The key of the map is the actual shader, we thus need to do a linear search to find our marked hash.
	for each (pair<ID3D10DeviceChild *, OriginalShaderInfo> iter in G->mReloadedShaders)
	{
		if (iter.second.hash == hash)
		{
			asmTextBlob = GetDisassembly(iter.second.byteCode);
			if (!asmTextBlob)
				break;

			// Disassembly file is written, now decompile the current byte code into HLSL.
			shaderModel = GetShaderModel(asmTextBlob);
			decompiled = Decompile(iter.second.byteCode, asmTextBlob);
			if (decompiled.empty())
				break;

			// Save the decompiled text, and ASM text into the .txt source file.
			if (!WriteHLSL(decompiled, asmTextBlob, hash, iter.second.shaderType))
				break;

			asmTextBlob->Release();

			// Lastly, reload the shader generated, to check for decompile errors, set it as the active 
			// shader code, in case there are visual errors, and make it the match the code in the file.
			wchar_t fileName[MAX_PATH];
			wsprintf(fileName, L"%08lx%08lx-%ls_replace.txt", (UINT32)(hash >> 32), (UINT32)(hash), iter.second.shaderType.c_str());
			if (!ReloadShader(G->SHADER_PATH, fileName, device))
				break;

			// There can be more than one in the map with the same hash, but we only need a single copy to
			// make the hlsl file output, so exit with success.
			success = true;
			break;
		}
	}

	if (success)
	{
		BeepSuccess();			// High beep for success, to notify it's running fresh fixes.
		LogInfo("> successfully copied Marked shader to ShaderFixes\n");
	}
	else
	{
		BeepFailure();			// Bonk sound for failure.
		LogInfo("> FAILED to copy Marked shader to ShaderFixes\n");
	}
}


// Key binding callbacks
static void TakeScreenShot(ID3D10Device *device, void *private_data)
{
	LogInfo("> capturing screenshot\n");

	HackerDevice* wrapped = (HackerDevice*)HackerDevice::m_List.GetDataPtr(device);
	if (wrapped->mStereoHandle)
	{
		NvAPI_Status err;
		err = NvAPI_Stereo_CapturePngImage(wrapped->mStereoHandle);
		if (err != NVAPI_OK)
		{
			LogInfo("> screenshot failed, error:%d\n", err);
			BeepFailure2();		// Brnk, dunk sound for failure.
		}
	}
}

static void ReloadFixes(ID3D10Device *device, void *private_data)
{
	LogInfo("> reloading *_replace.txt fixes from ShaderFixes\n");

	if (G->SHADER_PATH[0])
	{
		bool success = false;
		WIN32_FIND_DATA findFileData;
		wchar_t fileName[MAX_PATH];

		// Strict file name format, to allow renaming out of the way. "00aa7fa12bbf66b3-ps_replace.txt"
		// Will still blow up if the first characters are not hex.
		wsprintf(fileName, L"%ls\\????????????????-??_replace.txt", G->SHADER_PATH);
		HANDLE hFind = FindFirstFile(fileName, &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				success = ReloadShader(G->SHADER_PATH, findFileData.cFileName, device);
			} while (FindNextFile(hFind, &findFileData) && success);
			FindClose(hFind);
		}

		if (success)
		{
			BeepSuccess();		// High beep for success, to notify it's running fresh fixes.
			LogInfo("> successfully reloaded shaders from ShaderFixes\n");
		}
		else
		{
			BeepFailure();			// Bonk sound for failure.
			LogInfo("> FAILED to reload shaders from ShaderFixes\n");
		}
	}
}

static void DisableFix(ID3D10Device *device, void *private_data)
{
	LogInfo("show_original pressed - switching to original shaders\n");
	G->fix_enabled = false;
}

static void EnableFix(ID3D10Device *device, void *private_data)
{
	LogInfo("show_original released - switching to replaced shaders\n");
	G->fix_enabled = true;
}

static void NextIndexBuffer(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	auto i = G->mIndexBuffers.mVisited.find(G->mIndexBuffers.mSelected);
	if (i != G->mIndexBuffers.mVisited.end() && ++i != G->mIndexBuffers.mVisited.end())
	{
		G->mIndexBuffers.mSelected = *i;
		G->mIndexBuffers.mSelectedPos++;
		LogInfo("> traversing to next index buffer #%d. Number of index buffers in frame: %Iu\n", G->mIndexBuffers.mSelectedPos, G->mIndexBuffers.mVisited.size());
	}
	if (i == G->mIndexBuffers.mVisited.end() && ++G->mIndexBuffers.mSelectedPos < G->mIndexBuffers.mVisited.size() && G->mIndexBuffers.mSelectedPos >= 0)
	{
		i = G->mIndexBuffers.mVisited.begin();
		std::advance(i, G->mIndexBuffers.mSelectedPos);
		G->mIndexBuffers.mSelected = *i;
		LogInfo("> last index buffer lost. traversing to next index buffer #%d. Number of index buffers in frame: %Iu\n", G->mIndexBuffers.mSelectedPos, G->mIndexBuffers.mVisited.size());
	}
	if (i == G->mIndexBuffers.mVisited.end() && G->mIndexBuffers.mVisited.size() != 0)
	{
		G->mIndexBuffers.mSelectedPos = 0;
		LogInfo("> traversing to index buffer #0. Number of index buffers in frame: %Iu\n", G->mIndexBuffers.mVisited.size());

		G->mIndexBuffers.mSelected = *G->mIndexBuffers.mVisited.begin();
	}
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
}

static void PrevIndexBuffer(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	auto i = G->mIndexBuffers.mVisited.find(G->mIndexBuffers.mSelected);
	if (i != G->mIndexBuffers.mVisited.end() && i != G->mIndexBuffers.mVisited.begin())
	{
		--i;
		G->mIndexBuffers.mSelected = *i;
		G->mIndexBuffers.mSelectedPos--;
		LogInfo("> traversing to previous index buffer #%d. Number of index buffers in frame: %Iu\n", G->mIndexBuffers.mSelectedPos, G->mIndexBuffers.mVisited.size());
	}
	if (i == G->mIndexBuffers.mVisited.end() && --G->mIndexBuffers.mSelectedPos < G->mIndexBuffers.mVisited.size() && G->mIndexBuffers.mSelectedPos >= 0)
	{
		i = G->mIndexBuffers.mVisited.begin();
		std::advance(i, G->mIndexBuffers.mSelectedPos);
		G->mIndexBuffers.mSelected = *i;
		LogInfo("> last index buffer lost. traversing to previous index buffer #%d. Number of index buffers in frame: %Iu\n", G->mIndexBuffers.mSelectedPos, G->mIndexBuffers.mVisited.size());
	}
	if (i == G->mIndexBuffers.mVisited.end() && G->mIndexBuffers.mVisited.size() != 0)
	{
		G->mIndexBuffers.mSelectedPos = 0;
		LogInfo("> traversing to index buffer #0. Number of index buffers in frame: %Iu\n", G->mIndexBuffers.mVisited.size());

		G->mIndexBuffers.mSelected = *G->mIndexBuffers.mVisited.begin();
	}
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
}

static void MarkIndexBuffer(ID3D10Device *device, void *private_data)
{
	if (LogFile)
	{
		LogInfo(">>>> Index buffer marked: index buffer hash = %08lx%08lx\n", (UINT32)(G->mIndexBuffers.mSelected >> 32), (UINT32)G->mIndexBuffers.mSelected);
		for (auto i = G->mSelectedIndexBuffer_PixelShaders.begin(); i != G->mSelectedIndexBuffer_PixelShaders.end(); ++i)
			LogInfo("     visited pixel shader hash = %08lx%08lx\n", (UINT32)(*i >> 32), (UINT32)*i);
		for (auto i = G->mSelectedIndexBuffer_VertexShaders.begin(); i != G->mSelectedIndexBuffer_VertexShaders.end(); ++i)
			LogInfo("     visited vertex shader hash = %08lx%08lx\n", (UINT32)(*i >> 32), (UINT32)*i);
	}
	if (G->DumpUsage) DumpUsage();
}

static void NextPixelShader(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	auto i = G->mPixelShaders.mVisited.find(G->mPixelShaders.mSelected);
	if (i != G->mPixelShaders.mVisited.end() && ++i != G->mPixelShaders.mVisited.end())
	{
		G->mPixelShaders.mSelected = *i;
		G->mPixelShaders.mSelectedPos++;
		LogInfo("> traversing to next pixel shader #%d. Number of pixel shaders in frame: %Iu\n", G->mPixelShaders.mSelectedPos, G->mPixelShaders.mVisited.size());
	}
	if (i == G->mPixelShaders.mVisited.end() && ++G->mPixelShaders.mSelectedPos < G->mPixelShaders.mVisited.size() && G->mPixelShaders.mSelectedPos >= 0)
	{
		i = G->mPixelShaders.mVisited.begin();
		std::advance(i, G->mPixelShaders.mSelectedPos);
		G->mPixelShaders.mSelected = *i;
		LogInfo("> last pixel shader lost. traversing to next pixel shader #%d. Number of pixel shaders in frame: %Iu\n", G->mPixelShaders.mSelectedPos, G->mPixelShaders.mVisited.size());
	}
	if (i == G->mPixelShaders.mVisited.end() && G->mPixelShaders.mVisited.size() != 0)
	{
		G->mPixelShaders.mSelectedPos = 0;
		LogInfo("> traversing to pixel shader #0. Number of pixel shaders in frame: %Iu\n", G->mPixelShaders.mVisited.size());

		G->mPixelShaders.mSelected = *G->mPixelShaders.mVisited.begin();
	}
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
}

static void PrevPixelShader(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	auto i = G->mPixelShaders.mVisited.find(G->mPixelShaders.mSelected);
	if (i != G->mPixelShaders.mVisited.end() && i != G->mPixelShaders.mVisited.begin())
	{
		--i;
		G->mPixelShaders.mSelected = *i;
		G->mPixelShaders.mSelectedPos--;
		LogInfo("> traversing to previous pixel shader #%d. Number of pixel shaders in frame: %Iu\n", G->mPixelShaders.mSelectedPos, G->mPixelShaders.mVisited.size());
	}
	if (i == G->mPixelShaders.mVisited.end() && --G->mPixelShaders.mSelectedPos < G->mPixelShaders.mVisited.size() && G->mPixelShaders.mSelectedPos >= 0)
	{
		i = G->mPixelShaders.mVisited.begin();
		std::advance(i, G->mPixelShaders.mSelectedPos);
		G->mPixelShaders.mSelected = *i;
		LogInfo("> last pixel shader lost. traversing to previous pixel shader #%d. Number of pixel shaders in frame: %Iu\n", G->mPixelShaders.mSelectedPos, G->mPixelShaders.mVisited.size());
	}
	if (i == G->mPixelShaders.mVisited.end() && G->mPixelShaders.mVisited.size() != 0)
	{
		G->mPixelShaders.mSelectedPos = 0;
		LogInfo("> traversing to pixel shader #0. Number of pixel shaders in frame: %Iu\n", G->mPixelShaders.mVisited.size());

		G->mPixelShaders.mSelected = *G->mPixelShaders.mVisited.begin();
	}
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
}

static void MarkPixelShader(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	if (LogFile)
	{
		LogInfo(">>>> Pixel shader marked: pixel shader hash = %08lx%08lx\n", (UINT32)(G->mPixelShaders.mSelected >> 32), (UINT32)G->mPixelShaders.mSelected);
		for (auto i = G->mSelectedPixelShader_m_IndexBuffers.begin(); i != G->mSelectedPixelShader_m_IndexBuffers.end(); ++i)
			LogInfo("     visited index buffer hash = %08lx%08lx\n", (UINT32)(*i >> 32), (UINT32)*i);
		for (auto i = G->mPixelShaders.mInfo[G->mPixelShaders.mSelected].PartnerShaders.begin(); i != G->mPixelShaders.mInfo[G->mPixelShaders.mSelected].PartnerShaders.end(); ++i)
			LogInfo("     visited vertex shader hash = %08lx%08lx\n", (UINT32)(*i >> 32), (UINT32)*i);
	}
	auto i = G->mCompiledShaderMap.find(G->mPixelShaders.mSelected);
	if (i != G->mCompiledShaderMap.end())
	{
		LogInfo("       pixel shader was compiled from source code %s\n", i->second.c_str());
	}
	i = G->mCompiledShaderMap.find(G->mVertexShaders.mSelected);
	if (i != G->mCompiledShaderMap.end())
	{
		LogInfo("       vertex shader was compiled from source code %s\n", i->second.c_str());
	}
	// Copy marked shader to ShaderFixes
	CopyToFixes(G->mPixelShaders.mSelected, device);
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
	if (G->DumpUsage) DumpUsage();
}

static void NextVertexShader(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	auto i = G->mVertexShaders.mVisited.find(G->mVertexShaders.mSelected);
	if (i != G->mVertexShaders.mVisited.end() && ++i != G->mVertexShaders.mVisited.end())
	{
		G->mVertexShaders.mSelected = *i;
		G->mVertexShaders.mSelectedPos++;
		LogInfo("> traversing to next vertex shader #%d. Number of vertex shaders in frame: %Iu\n", G->mVertexShaders.mSelectedPos, G->mVertexShaders.mVisited.size());
	}
	if (i == G->mVertexShaders.mVisited.end() && ++G->mVertexShaders.mSelectedPos < G->mVertexShaders.mVisited.size() && G->mVertexShaders.mSelectedPos >= 0)
	{
		i = G->mVertexShaders.mVisited.begin();
		std::advance(i, G->mVertexShaders.mSelectedPos);
		G->mVertexShaders.mSelected = *i;
		LogInfo("> last vertex shader lost. traversing to previous vertex shader #%d. Number of vertex shaders in frame: %Iu\n", G->mVertexShaders.mSelectedPos, G->mVertexShaders.mVisited.size());
	}
	if (i == G->mVertexShaders.mVisited.end() && G->mVertexShaders.mVisited.size() != 0)
	{
		G->mVertexShaders.mSelectedPos = 0;
		LogInfo("> traversing to vertex shader #0. Number of vertex shaders in frame: %Iu\n", G->mVertexShaders.mVisited.size());

		G->mVertexShaders.mSelected = *G->mVertexShaders.mVisited.begin();
	}
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
}

static void PrevVertexShader(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	auto i = G->mVertexShaders.mVisited.find(G->mVertexShaders.mSelected);
	if (i != G->mVertexShaders.mVisited.end() && i != G->mVertexShaders.mVisited.begin())
	{
		--i;
		G->mVertexShaders.mSelected = *i;
		G->mVertexShaders.mSelectedPos--;
		LogInfo("> traversing to previous vertex shader #%d. Number of vertex shaders in frame: %Iu\n", G->mVertexShaders.mSelectedPos, G->mVertexShaders.mVisited.size());
	}
	if (i == G->mVertexShaders.mVisited.end() && --G->mVertexShaders.mSelectedPos < G->mVertexShaders.mVisited.size() && G->mVertexShaders.mSelectedPos >= 0)
	{
		i = G->mVertexShaders.mVisited.begin();
		std::advance(i, G->mVertexShaders.mSelectedPos);
		G->mVertexShaders.mSelected = *i;
		LogInfo("> last vertex shader lost. traversing to previous vertex shader #%d. Number of vertex shaders in frame: %Iu\n", G->mVertexShaders.mSelectedPos, G->mVertexShaders.mVisited.size());
	}
	if (i == G->mVertexShaders.mVisited.end() && G->mVertexShaders.mVisited.size() != 0)
	{
		G->mVertexShaders.mSelectedPos = 0;
		LogInfo("> traversing to vertex shader #0. Number of vertex shaders in frame: %Iu\n", G->mVertexShaders.mVisited.size());

		G->mVertexShaders.mSelected = *G->mVertexShaders.mVisited.begin();
	}
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
}

static void MarkVertexShader(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	if (LogFile)
	{
		LogInfo(">>>> Vertex shader marked: vertex shader hash = %08lx%08lx\n", (UINT32)(G->mVertexShaders.mSelected >> 32), (UINT32)G->mVertexShaders.mSelected);
		for (auto i = G->mVertexShaders.mInfo[G->mVertexShaders.mSelected].PartnerShaders.begin(); i != G->mVertexShaders.mInfo[G->mVertexShaders.mSelected].PartnerShaders.end(); ++i)
			LogInfo("     visited pixel shader hash = %08lx%08lx\n", (UINT32)(*i >> 32), (UINT32)*i);
		for (auto i = G->mSelectedVertexShader_m_IndexBuffers.begin(); i != G->mSelectedVertexShader_m_IndexBuffers.end(); ++i)
			LogInfo("     visited index buffer hash = %08lx%08lx\n", (UINT32)(*i >> 32), (UINT32)*i);
	}

	auto i = G->mCompiledShaderMap.find(G->mVertexShaders.mSelected);
	if (i != G->mCompiledShaderMap.end())
	{
		LogInfo("       shader was compiled from source code %s\n", i->second.c_str());
	}
	// Copy marked shader to ShaderFixes
	CopyToFixes(G->mVertexShaders.mSelected, device);
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
	if (G->DumpUsage) DumpUsage();
}

static void NextRenderTarget(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	auto i = G->mRenderTargets.mVisited.find(G->mRenderTargets.mSelected);
	if (i != G->mRenderTargets.mVisited.end() && ++i != G->mRenderTargets.mVisited.end())
	{
		G->mRenderTargets.mSelected = *i;
		G->mRenderTargets.mSelectedPos++;
		LogInfo("> traversing to next render target #%d. Number of render targets in frame: %Iu\n", G->mRenderTargets.mSelectedPos, G->mRenderTargets.mVisited.size());
	}
	if (i == G->mRenderTargets.mVisited.end() && ++G->mRenderTargets.mSelectedPos < G->mRenderTargets.mVisited.size() && G->mRenderTargets.mSelectedPos >= 0)
	{
		i = G->mRenderTargets.mVisited.begin();
		std::advance(i, G->mRenderTargets.mSelectedPos);
		G->mRenderTargets.mSelected = *i;
		LogInfo("> last render target lost. traversing to next render target #%d. Number of render targets frame: %Iu\n", G->mRenderTargets.mSelectedPos, G->mRenderTargets.mVisited.size());
	}
	if (i == G->mRenderTargets.mVisited.end() && G->mRenderTargets.mVisited.size() != 0)
	{
		G->mRenderTargets.mSelectedPos = 0;
		LogInfo("> traversing to render target #0. Number of render targets in frame: %Iu\n", G->mRenderTargets.mVisited.size());

		G->mRenderTargets.mSelected = *G->mRenderTargets.mVisited.begin();
	}
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
}

static void PrevRenderTarget(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	auto i = G->mRenderTargets.mVisited.find(G->mRenderTargets.mSelected);
	if (i != G->mRenderTargets.mVisited.end() && i != G->mRenderTargets.mVisited.begin())
	{
		--i;
		G->mRenderTargets.mSelected = *i;
		G->mRenderTargets.mSelectedPos--;
		LogInfo("> traversing to previous render target #%d. Number of render targets in frame: %Iu\n", G->mRenderTargets.mSelectedPos, G->mRenderTargets.mVisited.size());
	}
	if (i == G->mRenderTargets.mVisited.end() && --G->mRenderTargets.mSelectedPos < G->mRenderTargets.mVisited.size() && G->mRenderTargets.mSelectedPos >= 0)
	{
		i = G->mRenderTargets.mVisited.begin();
		std::advance(i, G->mRenderTargets.mSelectedPos);
		G->mRenderTargets.mSelected = *i;
		LogInfo("> last render target lost. traversing to previous render target #%d. Number of render targets in frame: %Iu\n", G->mRenderTargets.mSelectedPos, G->mRenderTargets.mVisited.size());
	}
	if (i == G->mRenderTargets.mVisited.end() && G->mRenderTargets.mVisited.size() != 0)
	{
		G->mRenderTargets.mSelectedPos = 0;
		LogInfo("> traversing to render target #0. Number of render targets in frame: %Iu\n", G->mRenderTargets.mVisited.size());

		G->mRenderTargets.mSelected = *G->mRenderTargets.mVisited.begin();
	}
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
}

static void LogRenderTarget(ID3D10Resource *target, char *log_prefix)
{
	char buf[256];

	if (!target || target == (void *)1) {
		LogInfo("No render target selected for marking\n");
		return;
	}

	UINT64 hash = G->mRenderTargets.mRegistered[target];
	struct ResourceInfo &info = G->mRenderTargets.mInfo[hash];
	StrRenderTarget(buf, 256, info);
	LogInfo("%srender target handle = %p, hash = %.16llx, %s\n",
		log_prefix, target, hash, buf);
}

static void MarkRenderTarget(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
	if (LogFile)
	{
		LogRenderTarget(G->mRenderTargets.mSelected, ">>>> Render target marked: ");
		for (auto i = G->mSelectedRenderTargetSnapshotList.begin(); i != G->mSelectedRenderTargetSnapshotList.end(); ++i)
		{
			LogRenderTarget(i->handle, "       ");
		}
	}
	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);

	if (G->DumpUsage) DumpUsage();
}

static void TuneUp(ID3D10Device *device, void *private_data)
{
	int index = (int)private_data;

	G->gTuneValue[index] += G->gTuneStep;
	LogInfo("> Value %i tuned to %f\n", index + 1, G->gTuneValue[index]);
}

static void TuneDown(ID3D10Device *device, void *private_data)
{
	int index = (int)private_data;

	G->gTuneValue[index] -= G->gTuneStep;
	LogInfo("> Value %i tuned to %f\n", index + 1, G->gTuneValue[index]);
}

// Start with a fresh set of shaders in the scene - either called explicitly
// via keypress, or after no hunting for 1 minute (see comment in RunFrameActions)
// Caller must have taken G->mCriticalSection (if enabled)
void TimeoutHuntingBuffers()
{
	G->mIndexBuffers.mVisited.clear();
	G->mVertexShaders.mVisited.clear();
	G->mPixelShaders.mVisited.clear();

	// FIXME: Not sure this is the right place to clear these - I think
	// they should be cleared every frame as they appear to be aimed at
	// providing a single frame usage snapshot on mark:
	G->mSelectedPixelShader_m_IndexBuffers.clear();
	G->mSelectedVertexShader_m_IndexBuffers.clear();
	G->mSelectedIndexBuffer_PixelShaders.clear();
	G->mSelectedIndexBuffer_VertexShaders.clear();

#if 0 /* Iterations are broken since we no longer use present() */
	// This seems totally bogus - shouldn't we be resetting the iteration
	// on each new frame, not after hunting timeout? This probably worked
	// back when RunFrameActions() was called from present(), but I suspect
	// has been broken ever since that was changed to come from draw(), and
	// it's not related to hunting buffers so it doesn't belong here:
	for (auto i = G->mShaderOverrideMap.begin(); i != G->mShaderOverrideMap.end(); ++i)
		i->second.iterations[0] = 0;
#endif
}

// User has requested all shaders be re-enabled
static void DoneHunting(ID3D10Device *device, void *private_data)
{
	if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);

	TimeoutHuntingBuffers();

	G->mRenderTargets.mSelectedPos = 0;
	G->mRenderTargets.mSelected = ((ID3D10Resource *)1),
		G->mPixelShaders.mSelected = 1;
	G->mPixelShaders.mSelectedPos = 0;
	G->mVertexShaders.mSelected = 1;
	G->mVertexShaders.mSelectedPos = 0;
	G->mIndexBuffers.mSelected = 1;
	G->mIndexBuffers.mSelectedPos = 0;

	if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
}

static void FlagConfigReload(ID3D10Device *device, void *private_data)
{
	// When we reload the configuration, we are going to clear the existing
	// key bindings and reassign them. Naturally this is not a safe thing
	// to do from inside a key binding callback, so we just set a flag and
	// do this after the input subsystem has finished dispatching calls.
	gReloadConfigPending = true;
}

void RegisterHuntingKeyBindings(wchar_t *iniFile)
{
	int i;
	wchar_t buf[16];
	int repeat = 8, noRepeat = 0;

	// reload_config is registered even if not hunting - this allows us to
	// turn on hunting in the ini dynamically without having to relaunch
	// the game. This can be useful in games that receive a significant
	// performance hit with hunting on, or where a broken effect is
	// discovered while playing normally where it may not be easy/fast to
	// find the effect again later.
	G->config_reloadable = RegisterIniKeyBinding(L"Hunting", L"reload_config", iniFile, FlagConfigReload, NULL, noRepeat, NULL);

	if (!G->hunting)
		return;

	if (GetPrivateProfileString(L"Hunting", L"repeat_rate", 0, buf, 16, iniFile))
		repeat = _wtoi(buf);

	RegisterIniKeyBinding(L"Hunting", L"next_pixelshader", iniFile, NextPixelShader, NULL, repeat, NULL);
	RegisterIniKeyBinding(L"Hunting", L"previous_pixelshader", iniFile, PrevPixelShader, NULL, repeat, NULL);
	RegisterIniKeyBinding(L"Hunting", L"mark_pixelshader", iniFile, MarkPixelShader, NULL, noRepeat, NULL);

	RegisterIniKeyBinding(L"Hunting", L"take_screenshot", iniFile, TakeScreenShot, NULL, noRepeat, NULL);

	RegisterIniKeyBinding(L"Hunting", L"next_indexbuffer", iniFile, NextIndexBuffer, NULL, repeat, NULL);
	RegisterIniKeyBinding(L"Hunting", L"previous_indexbuffer", iniFile, PrevIndexBuffer, NULL, repeat, NULL);
	RegisterIniKeyBinding(L"Hunting", L"mark_indexbuffer", iniFile, MarkIndexBuffer, NULL, noRepeat, NULL);

	RegisterIniKeyBinding(L"Hunting", L"next_vertexshader", iniFile, NextVertexShader, NULL, repeat, NULL);
	RegisterIniKeyBinding(L"Hunting", L"previous_vertexshader", iniFile, PrevVertexShader, NULL, repeat, NULL);
	RegisterIniKeyBinding(L"Hunting", L"mark_vertexshader", iniFile, MarkVertexShader, NULL, noRepeat, NULL);

	RegisterIniKeyBinding(L"Hunting", L"next_rendertarget", iniFile, NextRenderTarget, NULL, repeat, NULL);
	RegisterIniKeyBinding(L"Hunting", L"previous_rendertarget", iniFile, PrevRenderTarget, NULL, repeat, NULL);
	RegisterIniKeyBinding(L"Hunting", L"mark_rendertarget", iniFile, MarkRenderTarget, NULL, noRepeat, NULL);

	RegisterIniKeyBinding(L"Hunting", L"done_hunting", iniFile, DoneHunting, NULL, noRepeat, NULL);

	RegisterIniKeyBinding(L"Hunting", L"reload_fixes", iniFile, ReloadFixes, NULL, noRepeat, NULL);

	RegisterIniKeyBinding(L"Hunting", L"show_original", iniFile, DisableFix, EnableFix, noRepeat, NULL);

	for (i = 0; i < 4; i++) {
		_snwprintf(buf, 16, L"tune%i_up", i + 1);
		RegisterIniKeyBinding(L"Hunting", buf, iniFile, TuneUp, NULL, repeat, (void*)i);

		_snwprintf(buf, 16, L"tune%i_down", i + 1);
		RegisterIniKeyBinding(L"Hunting", buf, iniFile, TuneDown, NULL, repeat, (void*)i);
	}

	LogInfoW(L"  repeat_rate=%d\n", repeat);
}

// Rather than do all that, we now insert a RunFrameActions in the Draw method of the Context object,
// where it is absolutely certain that the game is fully loaded and ready to go, because it's actively
// drawing.  This gives us too many calls, maybe 5 per frame, but should not be a problem. The code
// is expecting to be called in a loop, and locks out auto-repeat using that looping.

// Draw is a very late binding for the game, and should solve all these problems, and allow us to retire
// the dxgi wrapper as unneeded.  The draw is caught at AfterDraw in the Context, which is called for
// every type of Draw, including DrawIndexed.

void RunFrameActions(ID3D10Device *device)
{
	static ULONGLONG last_ticks = 0;
	ULONGLONG ticks = GetTickCount64();

	// Prevent excessive input processing. XInput added an extreme
	// performance hit when processing four controllers on every draw call,
	// so only process input if at least 8ms has passed (approx 125Hz - may
	// be less depending on timer resolution)
	if (ticks - last_ticks < 8)
		return;
	last_ticks = ticks;

	LogDebug("Running frame actions.  Device: %p\n", device);

	// Regardless of log settings, since this runs every frame, let's flush the log
	// so that the most lost will be one frame worth.  Tradeoff of performance to accuracy
	if (LogFile) fflush(LogFile);

	bool newEvent = DispatchInputEvents(device);

	CurrentTransition.UpdateTransitions(device);

	// The config file is not safe to reload from within the input handler
	// since it needs to change the key bindings, so it sets this flag
	// instead and we handle it now.
	//if (ReloadConfigPending)
	//	ReloadConfig(device);

	// When not hunting most keybindings won't have been registered, but
	// still skip the below logic that only applies while hunting.
	if (!G->hunting)
		return;

	// Update the huntTime whenever we get fresh user input.
	if (newEvent)
		G->huntTime = time(NULL);

	// Clear buffers after some user idle time.  This allows the buffers to be
	// stable during a hunt, and cleared after one minute of idle time.  The idea
	// is to make the arrays of shaders stable so that hunting up and down the arrays
	// is consistent, while the user is engaged.  After 1 minute, they are likely onto
	// some other spot, and we should start with a fresh set, to keep the arrays and
	// active shader list small for easier hunting.  Until the first keypress, the arrays
	// are cleared at each thread wake, just like before. 
	// The arrays will be continually filled by the SetShader sections, but should 
	// rapidly converge upon all active shaders.

	if (difftime(time(NULL), G->huntTime) > 60) {
		if (G->ENABLE_CRITICAL_SECTION) EnterCriticalSection(&G->mCriticalSection);
		TimeoutHuntingBuffers();
		if (G->ENABLE_CRITICAL_SECTION) LeaveCriticalSection(&G->mCriticalSection);
	}
}

