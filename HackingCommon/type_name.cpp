#include "type_name.h"

#include <string>

// -----------------------------------------------------------------------------------------------
// When logging, it's not very helpful to have long sequences of hex instead of
// the actual names of the objects in question.
// e.g.
// DEFINE_GUID(IID_IDXGIFactory,0x7b7166ec,0x21c7,0x44ae,0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69);
//

std::string NameFromIID(IID id)
{
	// Adding every MIDL_INTERFACE from d3d11_1.h to make this reporting complete.
	// Doesn't seem useful to do every object from d3d11.h itself.

	if (__uuidof(IUnknown) == id)
		return "IUnknown";

	// FIXME: We should probably have these IIDs defined regardless of target
	// to catch potential cases where multiple versions of 3DMigoto are
	// coexisting and the devices get mixed up
#if MIGOTO_DX == 11
	if (IID_HackerDevice == id)
		return "HackerDevice";
	if (IID_HackerContext == id)
		return "HackerContext";
#elif MIGOTO_DX == 9
	// FIXME: DX9 GUIDs are not using the correct macros, and need verification
	// that they haven't been copy + pasted
	//if (IID_D3D9Wrapper_IDirect3DDevice9 == id)
	//	return "3DMigotoDevice9";
#endif

#ifdef _D3D9_H_
	if (__uuidof(IDirect3DDevice9) == id)
		return "IDirect3DDevice9";
#endif // _D3D9_H_

#ifdef __d3d10_h__
	if (__uuidof(ID3D10Multithread) == id)
		return "ID3D10Multithread";
	if (__uuidof(ID3D10Device) == id)
		return "ID3D10Device";
#endif // __d3d10_h__

#ifdef __d3d11_h__
	if (__uuidof(ID3D11Device) == id)
		return "ID3D11Device";
	if (__uuidof(ID3D11DeviceContext) == id)
		return "ID3D11DeviceContext";
	if (__uuidof(ID3D11DeviceChild) == id)
		return "ID3D11DeviceChild";
	if (__uuidof(ID3D11BlendState) == id)
		return "ID3D11BlendState";
	if (__uuidof(ID3D11RasterizerState) == id)
		return "ID3D11RasterizerState";
	if (__uuidof(ID3D11Texture2D) == id)	// Used to fetch backbuffer
		return "ID3D11Texture2D";
#endif // __d3d11_h__

#ifdef __d3d11_1_h__
	if (__uuidof(ID3D11BlendState1) == id)
		return "ID3D11BlendState1";
	if (__uuidof(ID3D11Device1) == id)
		return "ID3D11Device1";
	if (__uuidof(ID3D11DeviceContext1) == id)
		return "ID3D11DeviceContext1";
	if (__uuidof(ID3D11RasterizerState1) == id)
		return "ID3D11RasterizerState1";
	if (__uuidof(ID3DDeviceContextState) == id)
		return "ID3DDeviceContextState";
	if (__uuidof(ID3DUserDefinedAnnotation) == id)
		return "ID3DUserDefinedAnnotation";
#endif // __d3d11_1_h__

#ifdef __d3d11_2_h__
	// XXX: From newer Windows SDK than we are using. Defined in util.h for now
	if (__uuidof(ID3D11Device2) == id)  // d3d11_2.h when the time comes
		return "ID3D11Device2";
	if (__uuidof(ID3D11DeviceContext2) == id) // d3d11_2.h when the time comes
		return "ID3D11DeviceContext2";
#endif

#ifdef __d3d11sdklayers_h__
	if (__uuidof(ID3D11InfoQueue) == id)
		return "ID3D11InfoQueue";
#endif

	// All the DXGI interfaces from dxgi.h, and dxgi1_2.h
#ifdef __dxgi_h__
	if (__uuidof(IDXGIAdapter) == id)
		return "IDXGIAdapter";
	if (__uuidof(IDXGIAdapter1) == id)
		return "IDXGIAdapter1";
	if (__uuidof(IDXGIDevice) == id)
		return "IDXGIDevice";
	if (__uuidof(IDXGIDevice1) == id)
		return "IDXGIDevice1";
	if (__uuidof(IDXGIDeviceSubObject) == id)
		return "IDXGIDeviceSubObject";
	if (__uuidof(IDXGIFactory) == id)
		return "IDXGIFactory";
	if (__uuidof(IDXGIFactory1) == id)
		return "IDXGIFactory1";
	if (__uuidof(IDXGIKeyedMutex) == id)
		return "IDXGIKeyedMutex";
	if (__uuidof(IDXGIObject) == id)
		return "IDXGIObject";
	if (__uuidof(IDXGIOutput) == id)
		return "IDXGIOutput";
	if (__uuidof(IDXGIResource) == id)
		return "IDXGIResource";
	if (__uuidof(IDXGISurface) == id)
		return "IDXGISurface";
	if (__uuidof(IDXGISurface1) == id)
		return "IDXGISurface1";
	if (__uuidof(IDXGISwapChain) == id)
		return "IDXGISwapChain";
#endif // __dxgi_h__

#ifdef __dxgi1_2_h__
	if (__uuidof(IDXGIAdapter2) == id)
		return "IDXGIAdapter2";
	if (__uuidof(IDXGIDevice2) == id)
		return "IDXGIDevice2";
	if (__uuidof(IDXGIDisplayControl) == id)
		return "IDXGIDisplayControl";
	if (__uuidof(IDXGIFactory2) == id)
		return "IDXGIFactory2";
	if (__uuidof(IDXGIOutput1) == id)
		return "IDXGIOutput1";
	if (__uuidof(IDXGIOutputDuplication) == id)
		return "IDXGIOutputDuplication";
	if (__uuidof(IDXGIResource1) == id)
		return "IDXGIResource1";
	if (__uuidof(IDXGISurface2) == id)
		return "IDXGISurface2";
	if (__uuidof(IDXGISwapChain1) == id)
		return "IDXGISwapChain1";
#endif // __dxgi1_2_h__

	// XXX: From newer Windows SDK than we are using. Defined in util.h for now
#ifdef __dxgi1_3_h__
	if (__uuidof(IDXGISwapChain2) == id)		// dxgi1_3 A8BE2AC4-199F-4946-B331-79599FB98DE7
		return "IDXGISwapChain2";
#endif // __dxgi1_3_h__
#ifdef __dxgi1_4_h__
	if (__uuidof(IDXGISwapChain3) == id)		// dxgi1_4 94D99BDB-F1F8-4AB0-B236-7DA0170EDAB1
		return "IDXGISwapChain3";
#endif // __dxgi1_4_h__
#ifdef __dxgi1_5_h__
	if (__uuidof(IDXGISwapChain4) == id)		// dxgi1_5 3D585D5A-BD4A-489E-B1F4-3DBCB6452FFB
		return "IDXGISwapChain4";
#endif // __dxgi1_5_h__

	// For unknown IIDs lets return the hex string.
	// Converting from wchar_t to string using stackoverflow suggestion.

	std::string iidString;
	wchar_t wiid[128];
	if (SUCCEEDED(StringFromGUID2(id, wiid, 128)))
	{
		std::wstring convert = std::wstring(wiid);
		iidString = std::string(convert.begin(), convert.end());
	}
	else
	{
		iidString = "unknown";
	}

	return iidString;
}