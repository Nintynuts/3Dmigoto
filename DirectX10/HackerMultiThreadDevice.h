#pragma once

#include <windows.h>
#include "IDirect3DUnknown.h"
#include "D3D10Base.h"

#include "PointerSet.h"

    // MIDL_INTERFACE("9B7E4E00-342C-4106-A19F-4F2704F689F0")
class HackerMultithread : public IDirect3DUnknown
{
public:
    static ThreadSafePointerSet	 m_List;

    HackerMultithread(ID3D10Multithread* pDevice);

    __forceinline ID3D10Multithread* GetD3D10MultithreadDevice() { return (ID3D10Multithread*) m_pUnk; }

    static HackerMultithread* GetDirect3DMultithread(ID3D10Multithread* pDevice);

    // *** IDirect3DUnknown methods 
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

    // *** ID3D10Multithread methods
    STDMETHOD_(void, Enter)(THIS);

    STDMETHOD_(void, Leave)(THIS);

    STDMETHOD_(BOOL, SetMultithreadProtected)(THIS_
        /* [annotation] */
        __in  BOOL bMTProtect);

    STDMETHOD_(BOOL, GetMultithreadProtected)(THIS);
};