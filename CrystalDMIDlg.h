/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                            Copyright 2004-2008 hiyohiyo All rights reserved.
/*---------------------------------------------------------------------------*/

#if !defined(AFX_CRYSTALDMIDLG_H__30B33FA7_02D2_4668_92F1_5A2C48C3107A__INCLUDED_)
#define AFX_CRYSTALDMIDLG_H__30B33FA7_02D2_4668_92F1_5A2C48C3107A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct _DmiHeader
{
	UCHAR Type;
	UCHAR Length;
	WORD  Handle;
}DmiHeader;

/////////////////////////////////////////////////////////////////////////////
// CCrystalDMIDlg 

class CCrystalDMIDlg : public CDialog
{
public:
	CCrystalDMIDlg(CWnd* pParent = NULL);
	char m_ini[MAX_PATH];
	char m_path[MAX_PATH];
	//{{AFX_DATA(CCrystalDMIDlg)
	enum { IDD = IDD_CRYSTALDMI_DIALOG };
	CEdit	m_View;
	CComboBox m_Select;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CCrystalDMIDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:
	BOOL flag[129];
	CString c[130];
	HICON m_hIcon;
	HACCEL m_hAccelerator;
	HMODULE m_hOpenLibSys;
	BOOL m_IsNT;
	CString PrintHeader(DmiHeader* dmi);
	void Dump(DWORD address, DWORD size);
	//{{AFX_MSG(CCrystalDMIDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeSelect();
	afx_msg void OnCopyText();
	afx_msg void OnCrystalDewWorld();
	afx_msg void OnEnglish();
	afx_msg void OnExit();
	afx_msg void OnJapanese();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CRYSTALDMIDLG_H__30B33FA7_02D2_4668_92F1_5A2C48C3107A__INCLUDED_)
