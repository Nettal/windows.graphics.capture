//
// Created by Nettal on 2024/5/23.
//

#ifndef PARSEC_IDIRECT3DDXGIINTERFACEACCESS_H
#define PARSEC_IDIRECT3DDXGIINTERFACEACCESS_H

#include <rpc.h>
#include "windows.graphics.capture.interop.h"

typedef struct CIDirect3DDxgiInterfaceAccess CIDirect3DDxgiInterfaceAccess;
//"A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"
typedef struct CIDirect3DDxgiInterfaceAccessVtbl {
    BEGIN_INTERFACE

    /*** IUnknown methods ***/
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
            CIDirect3DDxgiInterfaceAccess *This,
            REFIID riid,
            void **ppvObject);

    ULONG (STDMETHODCALLTYPE *AddRef)(
            CIDirect3DDxgiInterfaceAccess *This);

    ULONG (STDMETHODCALLTYPE *Release)(
            CIDirect3DDxgiInterfaceAccess *This);

    /*** CIDirect3DDxgiInterfaceAccess methods ***/

    HRESULT (STDMETHODCALLTYPE *GetInterface)(
            CIDirect3DDxgiInterfaceAccess *This,
            REFIID iid,
            void **p
    );

    END_INTERFACE
} CIDirect3DDxgiInterfaceAccessVtbl;

struct CIDirect3DDxgiInterfaceAccess {
    CONST_VTBL struct CIDirect3DDxgiInterfaceAccessVtbl *lpVtbl;
};

#endif //PARSEC_IDIRECT3DDXGIINTERFACEACCESS_H
