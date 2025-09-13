/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                            Copyright 2004-2008 hiyohiyo All rights reserved.
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "crystal_dmi.h"
#include "crystal_dmi_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCrystalDMIApp

BEGIN_MESSAGE_MAP(CCrystalDMIApp, CWinApp)
	//{{AFX_MSG_MAP(CCrystalDMIApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCrystalDMIApp 

CCrystalDMIApp::CCrystalDMIApp()
{
}

/////////////////////////////////////////////////////////////////////////////

CCrystalDMIApp theApp;
int gLanguage = JAPANESE;

/////////////////////////////////////////////////////////////////////////////

BOOL CCrystalDMIApp::InitInstance()
{
	CCrystalDMIDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
