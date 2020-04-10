#pragma once

#include <d3d9.h>
#include <map>

static std::map<int, char*> D3DFORMATS = {
	{ 0, "UNKNOWN" },
	{ 20, "R8G8B8" },
	{ 21, "A8R8G8B8" },
	{ 22, "X8R8G8B8" },
	{ 23, "R5G6B5" },
	{ 24, "X1R5G5B5" },
	{ 25, "A1R5G5B5" },
	{ 26, "A4R4G4B4" },
	{ 27, "R3G3B2" },
	{ 28, "A8" },
	{ 29, "A8R3G3B2" },
	{ 30, "X4R4G4B4" },
	{ 31, "A2B10G10R10" },
	{ 32, "A8B8G8R8" },
	{ 33, "X8B8G8R8" },
	{ 34, "G16R16" },
	{ 35, "A2R10G10B10" },
	{ 36, "A16B16G16R16" },
	{ 40, "A8P8" },
	{ 41, "P8" },
	{ 50, "L8" },
	{ 51, "A8L8" },
	{ 52, "A4L4" },
	{ 60, "V8U8" },
	{ 61, "L6V5U5" },
	{ 62, "X8L8V8U8" },
	{ 63, "Q8W8V8U8" },
	{ 64, "V16U16" },
	{ 67, "A2W10V10U10" },
	{ 70, "D16_LOCKABLE" },
	{ 71, "D32" },
	{ 73, "D15S1" },
	{ 75, "D24S8" },
	{ 77, "D24X8" },
	{ 79, "D24X4S4" },
	{ 80, "D16" },
	{ 82, "D32F_LOCKABLE" },
	{ 83, "D24FS8" },
	{ 84, "D32_LOCKABLE" },
	{ 85, "S8_LOCKABLE" },
	{ 81, "L16" },
	{ 100, "VERTEXDATA" },
	{ 101, "INDEX16" },
	{ 102, "INDEX32" },
	{ 110, "Q16W16V16U16" },
	{ 111, "R16F" },
	{ 112, "G16R16F" },
	{ 113, "A16B16G16R16F" },
	{ 114, "R32F" },
	{ 115, "G32R32F" },
	{ 116, "A32B32G32R32F" },
	{ 117, "CxV8U8" },
	{ 118, "A1" },
	{ 119, "A2B10G10R10_XR_BIAS" },
	{ 199, "BINARYBUFFER " }
};

static char* TexFormatStrDX9(D3DFORMAT format)
{
	switch (format) {
	case MAKEFOURCC('U', 'Y', 'V', 'Y'):
		return "UYVY";
	case MAKEFOURCC('R', 'G', 'B', 'G'):
		return "R8G8_B8G8";
	case MAKEFOURCC('Y', 'U', 'Y', '2'):
		return "YUY2";
	case MAKEFOURCC('G', 'R', 'G', 'B'):
		return "G8R8_G8B8";
	case MAKEFOURCC('D', 'X', 'T', '1'):
		return "DXT1";
	case MAKEFOURCC('D', 'X', 'T', '2'):
		return "DXT2";
	case MAKEFOURCC('D', 'X', 'T', '3'):
		return "DXT3";
	case MAKEFOURCC('D', 'X', 'T', '4'):
		return "DXT4";
	case MAKEFOURCC('D', 'X', 'T', '5'):
		return "DXT5";
	case MAKEFOURCC('M', 'E', 'T', '1'):
		return "MULTI2_ARGB8";
	default:
		std::map<int, char*>::iterator it;
		it = D3DFORMATS.find(format);
		if (it != D3DFORMATS.end())
			return it->second;
		return "UNKNOWN";

	}
}

static D3DFORMAT ParseFormatStringDX9(const char* fmt, bool allow_numeric_format)
{
	size_t num_formats = D3DFORMATS.size();
	unsigned format;
	int nargs, end;

	if (allow_numeric_format) {
		// Try parsing format string as decimal:
		nargs = sscanf_s(fmt, "%u%n", &format, &end);
		if (nargs == 1 && end == strlen(fmt))
			return (D3DFORMAT)format;
	}

	if (!_strnicmp(fmt, "D3DFMT_", 7))
		fmt += 7;

	// Look up format string:
	std::map<int, char*>::iterator it;
	for (it = D3DFORMATS.begin(); it != D3DFORMATS.end(); it++)
	{
		if (!_strnicmp(fmt, it->second, 30))
			return (D3DFORMAT)it->first;
	}
	// UNKNOWN/0 is a valid format (e.g. for structured buffers), so return
	// -1 cast to a DXGI_FORMAT to signify an error:
	return (D3DFORMAT)-1;
}

static D3DFORMAT ParseFormatStringDX9(const wchar_t* wfmt, bool allow_numeric_format)
{
	char afmt[42];

	wcstombs(afmt, wfmt, 42);
	afmt[41] = '\0';

	return ParseFormatStringDX9(afmt, allow_numeric_format);
}
inline size_t BitsPerPixel(_In_ D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_A32B32G32R32F:
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		128;
	case D3DFMT_A16B16G16R16:
	case D3DFMT_Q16W16V16U16:
	case D3DFMT_A16B16G16R16F:
	case D3DFMT_G32R32F:
	case D3DFMT_DXT1:
		64;
	case D3DFMT_A2B10G10R10:
	case D3DFMT_A8B8G8R8:
	case D3DFMT_X8B8G8R8:
	case D3DFMT_G16R16:
	case D3DFMT_A2R10G10B10:
	case D3DFMT_V16U16:
	case D3DFMT_A2W10V10U10:
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_X8L8V8U8:
	case D3DFMT_Q8W8V8U8:
	case D3DFMT_D32:
	case D3DFMT_D24S8:
	case D3DFMT_D24X8:
	case D3DFMT_D24X4S4:
	case D3DFMT_D32F_LOCKABLE:
	case D3DFMT_D24FS8:
	case D3DFMT_D32_LOCKABLE:
	case D3DFMT_INDEX32:
	case D3DFMT_G16R16F:
	case D3DFMT_R32F:
	case D3DFMT_A2B10G10R10_XR_BIAS:
	case D3DFMT_UYVY:
	case D3DFMT_YUY2:
		32;
	case D3DFMT_R8G8B8:
		24;
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
	case D3DFMT_A8R3G3B2:
	case D3DFMT_X4R4G4B4:
	case D3DFMT_A8P8:
	case D3DFMT_A8L8:
	case D3DFMT_V8U8:
	case D3DFMT_L6V5U5:
	case D3DFMT_D16_LOCKABLE:
	case D3DFMT_D15S1:
	case D3DFMT_D16:
	case D3DFMT_L16:
	case D3DFMT_INDEX16:
	case D3DFMT_R16F:
	case D3DFMT_CxV8U8:
	case D3DFMT_R8G8_B8G8:
	case D3DFMT_G8R8_G8B8:
		16;
	case D3DFMT_R3G3B2:
	case D3DFMT_A8:
	case D3DFMT_P8:
	case D3DFMT_L8:
	case D3DFMT_A4L4:
	case D3DFMT_S8_LOCKABLE:
		8;
	default:
		return 0;
	}
}
static UINT d3d_format_bytes(D3DFORMAT format) {

	switch (format) {
	case D3DFMT_A32B32G32R32F:
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		16;
	case D3DFMT_A16B16G16R16:
	case D3DFMT_Q16W16V16U16:
	case D3DFMT_A16B16G16R16F:
	case D3DFMT_G32R32F:
	case D3DFMT_DXT1:
		8;
	case D3DFMT_A2B10G10R10:
	case D3DFMT_A8B8G8R8:
	case D3DFMT_X8B8G8R8:
	case D3DFMT_G16R16:
	case D3DFMT_A2R10G10B10:
	case D3DFMT_V16U16:
	case D3DFMT_A2W10V10U10:
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_X8L8V8U8:
	case D3DFMT_Q8W8V8U8:
	case D3DFMT_D32:
	case D3DFMT_D24S8:
	case D3DFMT_D24X8:
	case D3DFMT_D24X4S4:
	case D3DFMT_D32F_LOCKABLE:
	case D3DFMT_D24FS8:
	case D3DFMT_D32_LOCKABLE:
	case D3DFMT_INDEX32:
	case D3DFMT_G16R16F:
	case D3DFMT_R32F:
	case D3DFMT_A2B10G10R10_XR_BIAS:
	case D3DFMT_UYVY:
	case D3DFMT_YUY2:
		4;
	case D3DFMT_R8G8B8:
		3;
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
	case D3DFMT_A8R3G3B2:
	case D3DFMT_X4R4G4B4:
	case D3DFMT_A8P8:
	case D3DFMT_A8L8:
	case D3DFMT_V8U8:
	case D3DFMT_L6V5U5:
	case D3DFMT_D16_LOCKABLE:
	case D3DFMT_D15S1:
	case D3DFMT_D16:
	case D3DFMT_L16:
	case D3DFMT_INDEX16:
	case D3DFMT_R16F:
	case D3DFMT_CxV8U8:
	case D3DFMT_R8G8_B8G8:
	case D3DFMT_G8R8_G8B8:
		2;
	case D3DFMT_R3G3B2:
	case D3DFMT_A8:
	case D3DFMT_P8:
	case D3DFMT_L8:
	case D3DFMT_A4L4:
	case D3DFMT_S8_LOCKABLE:
		1;
	default:
		return 0;
	}

}
static UINT byteSizeFromD3DType(D3DDECLTYPE type) {
	switch (type) {
	case D3DDECLTYPE_FLOAT1:
		return sizeof(float);
	case D3DDECLTYPE_FLOAT2:
		return 2 * sizeof(float);
	case D3DDECLTYPE_FLOAT3:
		return 3 * sizeof(float);
	case D3DDECLTYPE_FLOAT4:
		return 4 * sizeof(float);
	case D3DDECLTYPE_D3DCOLOR:
	case D3DDECLTYPE_UBYTE4:
	case D3DDECLTYPE_UBYTE4N:
		return 4 * sizeof(BYTE);
	case D3DDECLTYPE_SHORT2:
	case D3DDECLTYPE_SHORT2N:
	case D3DDECLTYPE_USHORT2N:
	case D3DDECLTYPE_FLOAT16_2:
		return 2 * sizeof(short int);
	case D3DDECLTYPE_SHORT4:
	case D3DDECLTYPE_SHORT4N:
	case D3DDECLTYPE_USHORT4N:
	case D3DDECLTYPE_FLOAT16_4:
		return 4 * sizeof(short int);
	case D3DDECLTYPE_UDEC3:
	case D3DDECLTYPE_DEC3N:
		return 3 * sizeof(short int);
	case D3DDECLTYPE_UNUSED:
		return 0;
	default:
		return NULL;
	}
}

static DWORD decl_type_to_FVF(D3DDECLTYPE type, D3DDECLUSAGE usage, BYTE usageIndex, int nWeights) {
	switch (type) {
	case D3DDECLTYPE_FLOAT3:
		switch (usage) {
		case D3DDECLUSAGE_POSITION:
			return D3DFVF_XYZ;
		case D3DDECLUSAGE_NORMAL:
			return D3DFVF_NORMAL;
		default:
			return NULL;
		}
	case D3DDECLTYPE_FLOAT4:
		if (usage == D3DDECLUSAGE_POSITIONT)
			return D3DFVF_XYZRHW;
		return NULL;
	case D3DDECLTYPE_UBYTE4:
		if (usage == D3DDECLUSAGE_BLENDINDICES)
			switch (nWeights) {
			case 0:
				return D3DFVF_XYZB1;
			case 1:
				return D3DFVF_XYZB2;
			case 2:
				return D3DFVF_XYZB3;
			case 3:
				return D3DFVF_XYZB4;
			case 4:
				return D3DFVF_XYZB5;
			default:
				return D3DFVF_XYZB1;
			}
	case D3DDECLTYPE_FLOAT1:
		if (usage == D3DDECLUSAGE_PSIZE)
			return D3DFVF_PSIZE;
		return NULL;
	case D3DDECLTYPE_D3DCOLOR:
		if (usage == D3DDECLUSAGE_COLOR) {
			switch (usageIndex) {
			case 0:
				return D3DFVF_DIFFUSE;
			case 1:
				return D3DFVF_SPECULAR;
			default:
				return NULL;
			}
		}
		else {
			return NULL;
		}
	default:
		return NULL;

	}

}

static D3DDECLTYPE d3d_format_to_decl_type(D3DFORMAT format)
{
	switch (format) {
	case D3DFMT_A32B32G32R32F:
		return D3DDECLTYPE_FLOAT4;
	case D3DFMT_A16B16G16R16:
		return D3DDECLTYPE_SHORT4;
	case D3DFMT_Q16W16V16U16:
		return D3DDECLTYPE_SHORT4;
	case D3DFMT_A16B16G16R16F:
		return D3DDECLTYPE_FLOAT16_4;
	case D3DFMT_G32R32F:
		return D3DDECLTYPE_FLOAT2;
	case D3DFMT_A2B10G10R10:
		return D3DDECLTYPE_UDEC3;
	case D3DFMT_A8B8G8R8:
		return 	D3DDECLTYPE_UBYTE4;
	case D3DFMT_X8B8G8R8:
		return 	D3DDECLTYPE_UBYTE4;
	case D3DFMT_G16R16:
		return D3DDECLTYPE_USHORT2N;
	case D3DFMT_A2R10G10B10:
		return D3DDECLTYPE_UDEC3;
	case D3DFMT_V16U16:
		return D3DDECLTYPE_SHORT2;
	case D3DFMT_A2W10V10U10:
	case D3DFMT_A8R8G8B8:
		return D3DDECLTYPE_D3DCOLOR;
	case D3DFMT_X8R8G8B8:
		return D3DDECLTYPE_UBYTE4;
	case D3DFMT_X8L8V8U8:
		return D3DDECLTYPE_UBYTE4;
	case D3DFMT_Q8W8V8U8:
		return D3DDECLTYPE_UBYTE4;
	case D3DFMT_D32:
	case D3DFMT_D24S8:
	case D3DFMT_D24X8:
	case D3DFMT_D24X4S4:
	case D3DFMT_D32F_LOCKABLE:
	case D3DFMT_D24FS8:
	case D3DFMT_D32_LOCKABLE:
	case D3DFMT_INDEX32:
	case D3DFMT_G16R16F:
		return D3DDECLTYPE_FLOAT16_2;
	case D3DFMT_R32F:
	case D3DFMT_A2B10G10R10_XR_BIAS:
		return D3DDECLTYPE_UDEC3;
	case D3DFMT_R8G8B8:
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_A4R4G4B4:
	case D3DFMT_A8R3G3B2:
	case D3DFMT_X4R4G4B4:
	case D3DFMT_A8P8:
	case D3DFMT_A8L8:
	case D3DFMT_V8U8:
	case D3DFMT_L6V5U5:
	case D3DFMT_D16_LOCKABLE:
	case D3DFMT_D15S1:
	case D3DFMT_D16:
	case D3DFMT_L16:
	case D3DFMT_INDEX16:
	case D3DFMT_R16F:
	case D3DFMT_CxV8U8:
	case D3DFMT_R8G8_B8G8:
	case D3DFMT_G8R8_G8B8:
	case D3DFMT_R3G3B2:
	case D3DFMT_A8:
	case D3DFMT_P8:
	case D3DFMT_L8:
	case D3DFMT_A4L4:
	case D3DFMT_S8_LOCKABLE:
	default:
		return (D3DDECLTYPE)-1;
	}
}


static UINT strideForFVF(DWORD FVF) {
	UINT totalBytes = 0;

	if (FVF & D3DFVF_XYZ)
		totalBytes += 3 * sizeof(float);
	if (FVF & D3DFVF_XYZRHW)
		totalBytes += 4 * sizeof(float);
	if (FVF & D3DFVF_XYZW)
		totalBytes += 4 * sizeof(float);
	if (FVF & D3DFVF_XYZB5) {
		totalBytes += 8 * sizeof(float);
	}
	if (FVF & D3DFVF_LASTBETA_UBYTE4) {
		totalBytes += 8 * sizeof(float);
	}
	if (FVF & D3DFVF_LASTBETA_D3DCOLOR) {
		totalBytes += 8 * sizeof(float);
	}
	if (FVF & D3DFVF_XYZB4) {
		totalBytes += 7 * sizeof(float);
	}
	if (FVF & D3DFVF_XYZB3) {
		totalBytes += 6 * sizeof(float);
	}
	if (FVF & D3DFVF_XYZB2) {
		totalBytes += 5 * sizeof(float);
	}
	if (FVF & D3DFVF_XYZB1) {
		totalBytes += 4 * sizeof(float);
	}
	if (FVF & D3DFVF_NORMAL) {
		totalBytes += 3 * sizeof(float);
	}
	if (FVF & D3DFVF_PSIZE) {
		totalBytes += sizeof(float);
	}
	if (FVF & D3DFVF_DIFFUSE) {
		totalBytes += sizeof(float);
	}
	if (FVF & D3DFVF_SPECULAR) {
		totalBytes += sizeof(float);
	}

	for (int x = 1; x < 8; x++) {
		if (FVF & D3DFVF_TEXCOORDSIZE1(x)) {
			totalBytes += sizeof(float);
		}
		if (FVF & D3DFVF_TEXCOORDSIZE2(x)) {
			totalBytes += 2 * sizeof(float);
		}
		if (FVF & D3DFVF_TEXCOORDSIZE3(x)) {
			totalBytes += 3 * sizeof(float);
		}
		if (FVF & D3DFVF_TEXCOORDSIZE4(x)) {
			totalBytes += 4 * sizeof(float);
		}
	}

	return totalBytes;

}
static UINT DrawVerticesCountToPrimitiveCount(UINT vCount, D3DPRIMITIVETYPE pType) {

	switch (pType) {
	case D3DPT_POINTLIST:
		return vCount;
	case D3DPT_LINELIST:
		return vCount / 2;
	case D3DPT_LINESTRIP:
		return vCount - 1;
	case D3DPT_TRIANGLELIST:
		return vCount / 3;
	case D3DPT_TRIANGLESTRIP:
		return vCount - 2;
	case D3DPT_TRIANGLEFAN:
		return vCount - 2;
	case D3DPT_FORCE_DWORD:
		return vCount - 2;
	default:
		return vCount - 2;
	}


}
static UINT DrawPrimitiveCountToVerticesCount(UINT pCount, D3DPRIMITIVETYPE pType) {

	switch (pType) {
	case D3DPT_POINTLIST:
		return pCount;
	case D3DPT_LINELIST:
		return pCount * 2;
	case D3DPT_LINESTRIP:
		return pCount + 1;
	case D3DPT_TRIANGLELIST:
		return pCount * 3;
	case D3DPT_TRIANGLESTRIP:
		return pCount + 2;
	case D3DPT_TRIANGLEFAN:
		return pCount + 2;
	case D3DPT_FORCE_DWORD:
		return pCount + 2;
	default:
		return pCount + 2;
	}
}