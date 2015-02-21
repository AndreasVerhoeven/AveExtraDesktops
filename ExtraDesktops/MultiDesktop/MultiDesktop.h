

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Sun Oct 18 03:00:34 2009
 */
/* Compiler settings for .\MultiDesktop.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __MultiDesktop_h__
#define __MultiDesktop_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IAveMultiDesktopHost_FWD_DEFINED__
#define __IAveMultiDesktopHost_FWD_DEFINED__
typedef interface IAveMultiDesktopHost IAveMultiDesktopHost;
#endif 	/* __IAveMultiDesktopHost_FWD_DEFINED__ */


#ifndef __IAveShellBrowser_FWD_DEFINED__
#define __IAveShellBrowser_FWD_DEFINED__
typedef interface IAveShellBrowser IAveShellBrowser;
#endif 	/* __IAveShellBrowser_FWD_DEFINED__ */


#ifndef __AveMultiDesktopHost_FWD_DEFINED__
#define __AveMultiDesktopHost_FWD_DEFINED__

#ifdef __cplusplus
typedef class AveMultiDesktopHost AveMultiDesktopHost;
#else
typedef struct AveMultiDesktopHost AveMultiDesktopHost;
#endif /* __cplusplus */

#endif 	/* __AveMultiDesktopHost_FWD_DEFINED__ */


#ifndef __AveShellBrowser_FWD_DEFINED__
#define __AveShellBrowser_FWD_DEFINED__

#ifdef __cplusplus
typedef class AveShellBrowser AveShellBrowser;
#else
typedef struct AveShellBrowser AveShellBrowser;
#endif /* __cplusplus */

#endif 	/* __AveShellBrowser_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IAveMultiDesktopHost_INTERFACE_DEFINED__
#define __IAveMultiDesktopHost_INTERFACE_DEFINED__

/* interface IAveMultiDesktopHost */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAveMultiDesktopHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5B54637-9B96-443B-8839-B262DA2B7D62")
    IAveMultiDesktopHost : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IAveMultiDesktopHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAveMultiDesktopHost * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAveMultiDesktopHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAveMultiDesktopHost * This);
        
        END_INTERFACE
    } IAveMultiDesktopHostVtbl;

    interface IAveMultiDesktopHost
    {
        CONST_VTBL struct IAveMultiDesktopHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAveMultiDesktopHost_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAveMultiDesktopHost_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAveMultiDesktopHost_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAveMultiDesktopHost_INTERFACE_DEFINED__ */


#ifndef __IAveShellBrowser_INTERFACE_DEFINED__
#define __IAveShellBrowser_INTERFACE_DEFINED__

/* interface IAveShellBrowser */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAveShellBrowser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("10A60489-E132-4A45-9D1B-64E1CA5F19BF")
    IAveShellBrowser : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OnMessage( 
            /* [in] */ MSG *pMsg) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SaveState( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetShellViewWindow( 
            /* [retval][out] */ HWND *pHwnd) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAveShellBrowserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAveShellBrowser * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAveShellBrowser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAveShellBrowser * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *OnMessage )( 
            IAveShellBrowser * This,
            /* [in] */ MSG *pMsg);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IAveShellBrowser * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SaveState )( 
            IAveShellBrowser * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetShellViewWindow )( 
            IAveShellBrowser * This,
            /* [retval][out] */ HWND *pHwnd);
        
        END_INTERFACE
    } IAveShellBrowserVtbl;

    interface IAveShellBrowser
    {
        CONST_VTBL struct IAveShellBrowserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAveShellBrowser_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAveShellBrowser_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAveShellBrowser_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAveShellBrowser_OnMessage(This,pMsg)	\
    ( (This)->lpVtbl -> OnMessage(This,pMsg) ) 

#define IAveShellBrowser_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IAveShellBrowser_SaveState(This)	\
    ( (This)->lpVtbl -> SaveState(This) ) 

#define IAveShellBrowser_GetShellViewWindow(This,pHwnd)	\
    ( (This)->lpVtbl -> GetShellViewWindow(This,pHwnd) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAveShellBrowser_INTERFACE_DEFINED__ */



#ifndef __MultiDesktopLib_LIBRARY_DEFINED__
#define __MultiDesktopLib_LIBRARY_DEFINED__

/* library MultiDesktopLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_MultiDesktopLib;

EXTERN_C const CLSID CLSID_AveMultiDesktopHost;

#ifdef __cplusplus

class DECLSPEC_UUID("4F417BE4-88B0-4480-861D-FE1268675023")
AveMultiDesktopHost;
#endif

EXTERN_C const CLSID CLSID_AveShellBrowser;

#ifdef __cplusplus

class DECLSPEC_UUID("073066F3-AC9F-4DE9-AF5A-ACC933445303")
AveShellBrowser;
#endif
#endif /* __MultiDesktopLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


