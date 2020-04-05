#pragma once

#include <d3d10_1.h>
#include <ctime>
#include <vector>
#include <set>
#include <unordered_map>

#include "DirectXMath.h"
#include "DecompileHLSL.h"

#include <PointerSet.h>
#include <enum.h>
#include <globals_common.h>
#define ResourceSnapshot Snapshot<ID3D10Resource>
#define TargetResourceSelection TargetResourceSelection<ID3D10Resource*, ID3D10Resource, ResourceInfo>
#define ShaderResourceSelection_(type) ShaderResourceSelection<UINT64, ID3D10Resource, type>

using namespace std;

const int MARKING_MODE_SKIP = 0;
const int MARKING_MODE_MONO = 1;
const int MARKING_MODE_ORIGINAL = 2;
const int MARKING_MODE_ZERO = 3;

// Key is index/vertex buffer, value is hash key.
typedef unordered_map<ID3D10Buffer *, UINT64> DataBufferMap;

// Source compiled shaders.
typedef unordered_map<UINT64, string> CompiledShaderMap;

// Strategy: This OriginalShaderInfo record and associated map is to allow us to keep track of every
//	pixelshader and vertexshader that are compiled from hlsl text from the ShaderFixes
//	folder.  This keeps track of the original shader information using the ID3D10VertexShader*
//	or ID3D10PixelShader* as a master key to the key map.
//	We are using the base class of ID3D10DeviceChild* since both descend from that, and that allows
//	us to use the same structure for Pixel and Vertex shaders both.

// Info saved about originally overridden shaders passed in by the game in CreateVertexShader or
// CreatePixelShader that have been loaded as HLSL
//	shaderType is "vs" or "ps" or maybe later "gs" (type wstring for file name use)
//	shaderModel is only filled in when a shader is replaced.  (type string for old D3 API use)
//	linkage is passed as a parameter, seems to be rarely if ever used.
//	byteCode is the original shader byte code passed in by game, or recompiled by override.
//	timeStamp allows reloading/recompiling only modified shaders
//	replacement is either ID3D10VertexShader or ID3D10PixelShader
struct OriginalShaderInfo
{
	UINT64 hash;
	wstring shaderType;
	string shaderModel;
	//ID3D10ClassLinkage* linkage;
	ID3DBlob* byteCode;
	FILETIME timeStamp;
	ID3D10DeviceChild* replacement;
};

// Key is the overridden shader that was given back to the game at CreateVertexShader (vs or ps)
typedef unordered_map<ID3D10DeviceChild *, OriginalShaderInfo> ShaderReloadMap;

//typedef unordered_map<ID3D10HullShader *, UINT64> HullShaderMap;
//typedef unordered_map<ID3D10DomainShader *, UINT64> DomainShaderMap;
//typedef unordered_map<ID3D10ComputeShader *, UINT64> ComputeShaderMap;
typedef unordered_map<ID3D10GeometryShader *, UINT64> GeometryShaderMap;

enum class DepthBufferFilter {
	INVALID = -1,
	NONE,
	DEPTH_ACTIVE,
	DEPTH_INACTIVE,
};
static EnumName_t<wchar_t *, DepthBufferFilter> DepthBufferFilterNames[] = {
	{L"none", DepthBufferFilter::NONE},
	{L"depth_active", DepthBufferFilter::DEPTH_ACTIVE},
	{L"depth_inactive", DepthBufferFilter::DEPTH_INACTIVE},
	{NULL, DepthBufferFilter::INVALID} // End of list marker
};

struct ShaderOverride {
	float separation;
	float convergence;
	bool skip;
#if 0 /* Iterations are broken since we no longer use present() */
	vector<int> iterations; // Only for separation changes, not shaders.
#endif
	vector<UINT64> indexBufferFilter;
	DepthBufferFilter depth_filter;
	UINT64 partner_hash;

	ShaderOverride() :
		separation(FLT_MAX),
		convergence(FLT_MAX),
		skip(false),
		depth_filter(DepthBufferFilter::NONE),
		partner_hash(0)
	{}
};
typedef unordered_map<UINT64, struct ShaderOverride> ShaderOverrideMap;

struct TextureOverride {
	int stereoMode;
	int format;
#if 0 /* Iterations are broken since we no longer use present() */
	vector<int> iterations;
#endif

	TextureOverride() :
		stereoMode(-1),
		format(-1)
	{}
};
typedef unordered_map<UINT64, struct TextureOverride> TextureOverrideMap;

struct SwapChainInfo
{
	int width, height;
};

struct ResourceInfo
{
	D3D10_RESOURCE_DIMENSION type;
	union {
		D3D10_TEXTURE2D_DESC tex2d_desc;
		D3D10_TEXTURE3D_DESC tex3d_desc;
	};

	ResourceInfo() :
		type(D3D10_RESOURCE_DIMENSION_UNKNOWN)
	{}

	struct ResourceInfo & operator= (D3D10_TEXTURE2D_DESC desc)
	{
		type = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
		tex2d_desc = desc;
		return *this;
	}

	struct ResourceInfo & operator= (D3D10_TEXTURE3D_DESC desc)
	{
		type = D3D10_RESOURCE_DIMENSION_TEXTURE3D;
		tex3d_desc = desc;
		return *this;
	}
};

// Tracks info about specific resource instances:
struct ResourceHandleInfo
{
	D3D10_RESOURCE_DIMENSION type;
	uint32_t hash;
	uint32_t orig_hash;	// Original hash at the time of creation
	uint32_t data_hash;	// Just the data hash for track_texture_updates

	// TODO: If we are sure we understand all possible differences between
	// the original desc and that obtained by querying the resource we
	// probably don't need to store these. One possible difference is the
	// MipMaps field, which can be set to 0 at creation time to tell DX to
	// create the mip-maps, and I presume it will be filled in by the time
	// we query the desc. Most of the other fields shouldn't change, but
	// I'm not positive about all the misc flags. For now, storing this
	// copy is safer but wasteful.
	union {
		D3D10_TEXTURE2D_DESC desc2D;
		D3D10_TEXTURE3D_DESC desc3D;
	};

	ResourceHandleInfo() :
		type(D3D10_RESOURCE_DIMENSION_UNKNOWN),
		hash(0),
		orig_hash(0),
		data_hash(0)
	{}
};

typedef unordered_map<ID3D10Resource*, ResourceHandleInfo> ResourceMap;

struct Globals
{
	wchar_t SHADER_PATH[MAX_PATH];
	wchar_t SHADER_CACHE_PATH[MAX_PATH];

	int SCREEN_WIDTH;
	int SCREEN_HEIGHT;
	int SCREEN_REFRESH;
	int SCREEN_FULLSCREEN;
	int marking_mode;
	bool gForceStereo;
	bool gCreateStereoProfile;
	int gSurfaceCreateMode;
	int gSurfaceSquareCreateMode;

	bool hunting;
	bool fix_enabled;
	bool config_reloadable;
	time_t huntTime;

	int EXPORT_HLSL;		// 0=off, 1=HLSL only, 2=HLSL+OriginalASM, 3= HLSL+OriginalASM+recompiledASM
	bool EXPORT_SHADERS, EXPORT_FIXED, CACHE_SHADERS, PRELOAD_SHADERS, SCISSOR_DISABLE;
	DecompilerSettings decompiler_settings;
	//char ZRepair_DepthTextureReg1, ZRepair_DepthTextureReg2;
	//string ZRepair_DepthTexture1, ZRepair_DepthTexture2;
	//vector<string> ZRepair_Dependencies1, ZRepair_Dependencies2;
	//string ZRepair_ZPosCalc1, ZRepair_ZPosCalc2;
	//string ZRepair_PositionTexture, ZRepair_WorldPosCalc;
	//vector<string> InvTransforms;
	//string BackProject_Vector1, BackProject_Vector2;
	//string ObjectPos_ID1, ObjectPos_ID2, ObjectPos_MUL1, ObjectPos_MUL2;
	//string MatrixPos_ID1, MatrixPos_MUL1;
	UINT64 ZBufferHashToInject;
	//bool FIX_SV_Position;
	//bool FIX_Light_Position;
	//bool FIX_Recompile_VS;
	bool DumpUsage;
	bool ENABLE_TUNE;
	float gTuneValue[4], gTuneStep;

	DirectX::XMFLOAT4 iniParams;

	SwapChainInfo mSwapChainInfo;

	ThreadSafePointerSet m_AdapterList;
	CRITICAL_SECTION mCriticalSection;
	bool ENABLE_CRITICAL_SECTION;

	CRITICAL_SECTION mResourcesLock;
	ResourceMap mResources;

	DataBufferMap mDataBuffers;

	ResourceSelection<UINT64> mIndexBuffers;

	set<UINT64> mSelectedIndexBuffer_VertexShaders; // set so that shaders used with an index buffer will be sorted in log when marked
	set<UINT64> mSelectedIndexBuffer_PixelShaders; // set so that shaders used with an index buffer will be sorted in log when marked

	CompiledShaderMap mCompiledShaderMap;	

	ShaderResourceSelection_(ID3D10VertexShader) mVertexShaders;

	set<UINT64> mSelectedVertexShader_m_IndexBuffers; // set so that index buffers used with a shader will be sorted in log when marked

	ShaderResourceSelection_(ID3D10PixelShader) mPixelShaders;

	set<UINT64> mSelectedPixelShader_m_IndexBuffers; // set so that index buffers used with a shader will be sorted in log when marked

	ShaderReloadMap mReloadedShaders;						// Shaders that were reloaded live from ShaderFixes

	GeometryShaderMap mGeometryShaders;
	//ComputeShaderMap mComputeShaders;
	//DomainShaderMap mDomainShaders;
	//HullShaderMap mHullShaders;

	ShaderOverrideMap mShaderOverrideMap;
	TextureOverrideMap mTextureOverrideMap;

	// Statistics
	TargetResourceSelection mRenderTargets;
	vector<ResourceSnapshot> mCurrentRenderTargets;

	ID3D10Resource *mCurrentDepthTarget;
	map<UINT64, struct ResourceInfo> mDepthTargetInfo; // map so that ShaderUsage.txt is sorted - lookup time is O(log N)
	// Snapshot of all targets for selection.
	ID3D10Resource* mSelectedRenderTargetSnapshot;
	set<ResourceSnapshot> mSelectedRenderTargetSnapshotList; // set so that render targets will be sorted in log when marked
	// Relations
	unordered_map<ID3D10Texture2D *, UINT64> mTexture2D_ID;
	unordered_map<ID3D10Texture3D *, UINT64> mTexture3D_ID;

	Globals() :
		mSelectedRenderTargetSnapshot(0),
		mCurrentDepthTarget(0),

		hunting(false),
		fix_enabled(true),
		config_reloadable(false),
		huntTime(0),

		EXPORT_SHADERS(false),
		EXPORT_HLSL(0),
		EXPORT_FIXED(false),
		CACHE_SHADERS(false),
		PRELOAD_SHADERS(false),
		DumpUsage(false),
		ENABLE_TUNE(false),
		gTuneStep(0.001f),

		iniParams{ FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX },

		ENABLE_CRITICAL_SECTION(false),
		SCREEN_WIDTH(-1),
		SCREEN_HEIGHT(-1),
		SCREEN_REFRESH(-1),
		SCREEN_FULLSCREEN(-1),
		marking_mode(-1),
		gForceStereo(false),
		gCreateStereoProfile(false),
		gSurfaceCreateMode(-1),
		gSurfaceSquareCreateMode(-1),
		SCISSOR_DISABLE(0)
	{
		SHADER_PATH[0] = 0; 
		SHADER_CACHE_PATH[0] = 0;
		mSwapChainInfo.width = -1;
		mSwapChainInfo.height = -1;

		for (int i = 0; i < 4; i++)
			gTuneValue[i] = 1.0f;
	}
};

extern Globals *G;

static inline ResourceMap::iterator lookup_resource_handle_info(ID3D10Resource* resource)
{
	return G->mResources.find(resource);
}