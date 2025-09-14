/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                            Copyright 2004-2009 hiyohiyo All rights reserved.
/*---------------------------------------------------------------------------*/

#if !defined(AFX_CRYSTALDMI_H__E951D4CF_8F1E_40F0_820E_A968447F507B__INCLUDED_)
#define AFX_CRYSTALDMI_H__E951D4CF_8F1E_40F0_820E_A968447F507B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"

////////////
// Language
//////////////////////////////////////////

#define		JAPANESE				0x0001
#define		ENGLISH					0x0002

#ifdef _X86_64
#define		CRYSTAL_DMI_PRODUCT		_T("CrystalDMI Pure x64 Edition")
#define		CRYSTAL_DMI_STATUS		_T("") // Alpha, Beta...
#else
#define		CRYSTAL_DMI_PRODUCT		_T("CrystalDMI")
#define		CRYSTAL_DMI_STATUS		_T("") // Alpha, Beta...
#endif

#define		CRYSTAL_DMI_VERSION		_T("1.0.6.1")

////////////
// URL
//////////////////////////////////////////

#define URL_JAPANESE	_T("http://crystalmark.info/")
#define URL_ENGLISH		_T("http://crystalmark.info/?lang=en")

/////////////////////////////////////////////////////////////////////////////

class CCrystalDMIApp : public CWinApp
{
public:
	CCrystalDMIApp();

	//{{AFX_VIRTUAL(CCrystalDMIApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CCrystalDMIApp)
	afx_msg void OnCopyText();
	afx_msg void OnCrystalDewWorld();
	afx_msg void OnEnglish();
	afx_msg void OnJapanese();
	afx_msg void OnLicense();
	afx_msg void OnReadMe();
	afx_msg void OnExit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CRYSTALDMI_H__E951D4CF_8F1E_40F0_820E_A968447F507B__INCLUDED_)
