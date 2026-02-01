// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__CCBDBBD7_8D59_4376_A050_962ADAC82FA1__INCLUDED_)
#define AFX_STDAFX_H__CCBDBBD7_8D59_4376_A050_962ADAC82FA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WINVER 0x0501
#define _INT8_T_DEFINED
#define _UINT32_T_DEFINED

#define NOMINMAX
#define _GXNOMSG
#define _SFLNOMSG
#define _SECNOMSG

#define VC_EXTRALEAN    // Exclude rarely-used stuff from Windows headers

/*
#ifdef _DEBUG
#define _STLP_DEBUG
#define _STLP_DEBUG_ALLOC
#else
#ifndef NDEBUG
#define NDEBUG
#endif
#endif
*/

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>   // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxole.h>
#include <afxdao.h>
#include <atldbcli.h>

#pragma warning(disable: 4786)


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CCBDBBD7_8D59_4376_A050_962ADAC82FA1__INCLUDED_)
