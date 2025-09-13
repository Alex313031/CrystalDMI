/*-----------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                              Copyright 2004-2010 hiyohiyo All rights reserved.
/*-----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "crystal_dmi.h"
#include "crystal_dmi_dlg.h"

#include "ols_def.h"
#include "ols_api_init.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	ALL_INFORMATION_ID 129

extern int gLanguage;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
static BOOL IsNT();

/////////////////////////////////////////////////////////////////////////////
// CCrystalDMIDlg Dialog

CCrystalDMIDlg::CCrystalDMIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCrystalDMIDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCrystalDMIDlg)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCrystalDMIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCrystalDMIDlg)
	DDX_Control(pDX, IDC_VIEW, m_View);
	DDX_Control(pDX, IDC_SELECT, m_Select);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCrystalDMIDlg, CDialog)
	//{{AFX_MSG_MAP(CCrystalDMIDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_SELECT, &CCrystalDMIDlg::OnSelchangeSelect)
	ON_COMMAND(IDM_COPY_TEXT, &CCrystalDMIDlg::OnCopyText)
	ON_COMMAND(IDM_CRYSTAL_DEW_WORLD, &CCrystalDMIDlg::OnCrystalDewWorld)
	ON_COMMAND(IDM_ENGLISH, &CCrystalDMIDlg::OnEnglish)
	ON_COMMAND(IDM_EXIT, &CCrystalDMIDlg::OnExit)
	ON_COMMAND(IDM_JAPANESE, &CCrystalDMIDlg::OnJapanese)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define MAP_MEMORY_SIZE 64*1024

/////////////////////////////////////////////////////////////////////////////
// String 

static CString DmiString(DmiHeader* dmi, UCHAR id, BOOL rn = TRUE)
{
	static CString cstr;
	char *p = (char *)dmi;

	p += dmi->Length;

	while(id > 1 && *p)
	{
		p += strlen(p);
		p++;
		id--;
	}
	// ASCII Filter
	for(DWORD i = 0; i < strlen(p); i++){
		if(p[i] < 32 || p[i] == 127){
			p[i]='.';
		}
	}
	cstr = p;
	if(rn){
		cstr += _T("\r\n");
	}

	return cstr;
}

static CString DmiStringB(BYTE b, BOOL rn = TRUE)
{
	static CString cstr;
	cstr.Format(_T("%d"), b);
	if(rn){
		cstr += _T("\r\n");
	}
	return cstr;
}

static CString DmiStringW(WORD w, BOOL rn = TRUE)
{
	static CString cstr;
	cstr.Format(_T("%d"), w);
	if(rn){
		cstr += _T("\r\n");
	}
	return cstr;
}

static CString DmiStringD(DWORD d, BOOL rn = TRUE)
{
	static CString cstr;
	cstr.Format(_T("%d"), d);
	if(rn){
		cstr += _T("\r\n");
	}
	return cstr;
}

static CString DmiStringBX(BYTE b, DWORD type = 0, BOOL rn = TRUE)
{
	static CString cstr;
	switch (type)
	{
	case 0:
		cstr.Format(_T("%02Xh (%d)"), b, b);
		break;
	case 1:
		cstr.Format(_T("%02X"), b);
		break;
	case 2:
		cstr.Format(_T("%02Xh"), b);
		break;
	}
	if(rn){
		cstr += _T("\r\n");
	}
	return cstr;
}

static CString DmiStringWX(WORD w, DWORD type = 0, BOOL rn = TRUE)
{
	static CString cstr;
	switch (type)
	{
	case 0:
		cstr.Format(_T("%04Xh (%d)"), w, w);
		break;
	case 1:
		cstr.Format(_T("%04X"), w);
		break;
	case 2:
		cstr.Format(_T("%04Xh"), w);
		break;
	case 3:
		if(w >= 0xFFFE){
			cstr.Format(_T("N/A"));
		}else{
			cstr.Format(_T("%04Xh"),w);
		}
		break;
	}
	if(rn){
		cstr += _T("\r\n");
	}
	return cstr;
}

static CString DmiStringDX(DWORD d, DWORD type = 0, BOOL rn = TRUE)
{
	static CString cstr;
	switch (type)
	{
	case 0:
		cstr.Format(_T("%08Xh (%d)"), d, d);
		break;
	case 1:
		cstr.Format(_T("%08X"), d);
		break;
	case 2:
		cstr.Format(_T("%08Xh"), d);
		break;
	}
	if(rn){
		cstr += _T("\r\n");
	}
	return cstr;
}

static BOOL CheckSum(const BYTE *buf, int length)
{
	BYTE sum = 0;
	
	for(int i = 0; i < length; i++){
		sum += buf[i];
	}
	return (sum==0);
}


CString CCrystalDMIDlg::PrintHeader(DmiHeader* dmi)
{
	CString cstr;
	char *p = (char *)dmi;
	if(flag[p[0] + 1] == FALSE){
		c[p[0] + 1] += "\r\n";
	}
	cstr  = "Type                           : " + DmiStringBX(p[0]);
	cstr += "Length                         : " + DmiStringBX(p[1]);
	cstr += "Handle                         : " + DmiStringWX(MAKEWORD(p[2], p[3]), 3);
	
	return cstr;
}

//#define MAKEWORD(a,b)		((b<<8)+a))
#define MAKEDWORD(a,b,c,d)	((d<<24)+(c<<16)+(b<<8)+(a))

/////////////////////////////////////////////////////////////////////////////
// CCrystalDMIDlg 
BOOL CCrystalDMIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_hAccelerator = ::LoadAccelerators(AfxGetInstanceHandle(),
		                                MAKEINTRESOURCE(IDR_ACCELERATOR));


	m_IsNT = IsNT();

	m_hOpenLibSys = NULL;
	if(InitOpenLibSys(&m_hOpenLibSys) != TRUE)
	{
		AfxMessageBox(_T("DLL Load Error!!"));
		EndDialog(0);
		return TRUE;
	}

	switch(GetDllStatus())
	{
	case OLS_DLL_NO_ERROR:
		break;
	case OLS_DLL_UNSUPPORTED_PLATFORM:
		AfxMessageBox(_T("DLL Status Error!! UNSUPPORTED_PLATFORM"));
		EndDialog(0);
		return FALSE;
		break;
	case OLS_DLL_DRIVER_NOT_LOADED:
		AfxMessageBox(_T("DLL Status Error!! DRIVER_NOT_LOADED"));
		EndDialog(0);
		return FALSE;
		break;
	case OLS_DLL_DRIVER_NOT_FOUND:
		AfxMessageBox(_T("DLL Status Error!! DRIVER_NOT_FOUND"));
		EndDialog(0);
		return FALSE;
		break;
	case OLS_DLL_DRIVER_UNLOADED:
		AfxMessageBox(_T("DLL Status Error!! DRIVER_UNLOADED"));
		EndDialog(0);
		return FALSE;
		break;
	case OLS_DLL_DRIVER_NOT_LOADED_ON_NETWORK:
		AfxMessageBox(_T("DLL Status Error!! DRIVER_NOT_LOADED_ON_NETOWRK"));
		EndDialog(0);
		return FALSE;
		break;
	case OLS_DLL_UNKNOWN_ERROR:
	default:
		AfxMessageBox(_T("DLL Status Error!! UNKNOWN_ERROR"));
		EndDialog(0);
		return FALSE;
		break;
	}

	wchar_t str[256];
	CString cstr;
	UCHAR *d = NULL;
// Init m_path & m_ini 
	wchar_t* ptrEnd;
	::GetModuleFileName(NULL, m_path, MAX_PATH);
	::GetModuleFileName(NULL, m_ini, MAX_PATH);
	if ((ptrEnd = wcsrchr(m_path, _T('\\'))) != NULL){
		*ptrEnd = '\0';
	}
	if ((ptrEnd = wcsrchr(m_ini, _T('.'))) != NULL){
		*ptrEnd = '\0';
		_tcscat_s(m_ini, MAX_PATH, _T(".ini"));
	}
	
// Set Language
	GetPrivateProfileString(_T("Setting"), _T("Language"), _T("en"), str, 256, m_ini);
	if(wcscmp(str, _T("en")) == 0){
		gLanguage = ENGLISH;
	}else{
		gLanguage = JAPANESE;
	}

	if(gLanguage == ENGLISH){
		OnEnglish();
	}else{
		OnJapanese();
	}

	cstr.Format(_T("%s %s%s"), CRYSTAL_DMI_PRODUCT, CRYSTAL_DMI_VERSION, CRYSTAL_DMI_STATUS);
	SetWindowText(cstr);

	UCHAR b[MAP_MEMORY_SIZE];

	if(ReadPhysicalMemory(0x000F0000, b, MAP_MEMORY_SIZE, sizeof(UCHAR)) == 0){
		MessageBox(_T("FAILED ReadPhysicalMemory."));
		EndDialog(0);
		return FALSE;
	}

	try{

	CString cstr;
	int i;
	int j;
	UCHAR *p;
	UCHAR *next;
	WORD word;
	DWORD dword;
	DWORD EntryPoint;
	DWORD StructureLength;
	DWORD NumberOfStructures;
	BOOL Flag = FALSE;
	p = (UCHAR *)b;
	for(j = 0; j < MAP_MEMORY_SIZE; j += 16)
	{
		if(memcmp(p, "_SM_", 4) == 0){
			c[0]  = "SMBIOS Version                 : " + DmiStringB(p[6], 0) + "." + DmiStringB(p[7]);
			c[0] += "----------------------------------------------------------------------\r\n";
			c[0] += "Anchor String                  : _SM_\r\n";
			c[0] += "Entry Point Structure Checksum : " + DmiStringBX(p[4]);
			c[0] += "SMBIOS Major Version           : " + DmiStringBX(p[6]);
			c[0] += "SMBIOS Minor Version           : " + DmiStringBX(p[7]);
			c[0] += "Maximum Structure Size         : " + DmiStringWX(MAKEWORD(p[8], p[9]));
			c[0] += "Entry Point Revision           : " + DmiStringBX(p[0xA]);
			c[0] += "Formatted Area                 : " + DmiStringBX(p[0xB], 1, 0) + " "
														+ DmiStringBX(p[0xC], 1, 0) + " "
														+ DmiStringBX(p[0xD], 1, 0) + " "
														+ DmiStringBX(p[0xE], 1, 0) + " "
														+ DmiStringBX(p[0xF], 1);
		//	Flag = TRUE;
		}
		if(memcmp(p, "_DMI_", 5) == 0){
			EntryPoint = MAKEDWORD(p[8], p[9], p[0xA], p[0xB]);
			StructureLength = MAKEWORD(p[6], p[7]);
			NumberOfStructures = MAKEWORD(p[0xC], p[0xD]);
			c[0] += "Intermediate anchor string     : _DMI_\r\n";
			c[0] += "Intermediate Checksum          : " + DmiStringBX(p[5]);
			c[0] += "Structure Table Length         : " + DmiStringWX(MAKEWORD(p[6], p[7]));
			c[0] += "Structure Table Address        : " + DmiStringDX(MAKEDWORD(p[8], p[9], p[0xA], p[0xB]), 2);
			c[0] += "Number of SMBIOS Structures    : " + DmiStringWX(MAKEWORD(p[0xC], p[0xD]));
			c[0] += "SMBIOS BCD Revision            : " + DmiStringBX(p[0xE], 2);

			if(CheckSum(p, 0xF)){
				Flag = TRUE;
			}
			m_Select.AddString(_T(" SMBIOS/DMI Information"));
			break;
		}
		p+=16;
	}

	if(Flag == FALSE){
		MessageBox(_T("Not Found SMBIOS/DMI Information..."));
		EndDialog(0);
		return FALSE;
	}

	d = new UCHAR[StructureLength];
	ReadPhysicalMemory(EntryPoint, d, StructureLength, sizeof(UCHAR));

	DWORD num;
	WORD processorFamily; //
	DWORD ch; // Characteristics
	p = d;
	DmiHeader* dmi;
	for(i = 0; i < 128; i++){
		flag[i] = TRUE;
	};

	for(num = 0; num < NumberOfStructures; num++){
		dmi = (DmiHeader*)p;
		switch (dmi->Type){
///////////////////////////////////////////////////////////////////////////////////////////////////
// 00 BIOS Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 0:
			if(! flag[1]){c[1] += "\r\n";}
			c[1] += PrintHeader(dmi);
			c[1] += "Vendor                         : " + DmiString(dmi, p[4]);
			c[1] += "BIOS Version                   : " + DmiString(dmi, p[5]);
			c[1] += "BIOS Starting Address Segment  : " + DmiStringWX(MAKEWORD(p[6], p[7]), 2, 1);
			c[1] += "BIOS Release Date              : " + DmiString(dmi, p[8]);
			cstr.Format(_T("%d KB\r\n"), ((p[9] + 1) << 16 ) / 1024);
			c[1] += "BIOS ROM Size                  : " + cstr;
			c[1] += "BIOS Characteristics           : " + DmiStringBX(p[0x11], 1, 0)
														+ DmiStringBX(p[0x10], 1, 0)
														+ DmiStringBX(p[0xF], 1, 0)
														+ DmiStringBX(p[0xE], 1, 0)
														+ DmiStringBX(p[0xD], 1, 0)
														+ DmiStringBX(p[0xC], 1, 0)
														+ DmiStringBX(p[0xB], 1, 0)
														+ DmiStringBX(p[0xA], 1);
			ch = MAKEDWORD(p[0xA], p[0xB], p[0xC], p[0xD]);
			if((ch >>  0) & 0x1){c[1] += " - ""Reserved""\r\n";}
			if((ch >>  1) & 0x1){c[1] += " - ""Reserved""\r\n";}
			if((ch >>  2) & 0x1){c[1] += " - ""Unknown""\r\n";}
			if((ch >>  3) & 0x1){c[1] += " - ""BIOS Characteristics Not Supported""\r\n";}
			if((ch >>  4) & 0x1){c[1] += " - ""ISA is supported""\r\n";}
			if((ch >>  5) & 0x1){c[1] += " - ""MCA is supported""\r\n";}
			if((ch >>  6) & 0x1){c[1] += " - ""EISA is supported""\r\n";}
			if((ch >>  7) & 0x1){c[1] += " - ""PCI is supported""\r\n";}
			if((ch >>  8) & 0x1){c[1] += " - ""PC Card (PCMCIA) is supported""\r\n";}
			if((ch >>  9) & 0x1){c[1] += " - ""Plug and Play is supported""\r\n";}
			if((ch >> 10) & 0x1){c[1] += " - ""APM is supported""\r\n";}
			if((ch >> 11) & 0x1){c[1] += " - ""BIOS is Upgradeable (Flash)""\r\n";}
			if((ch >> 12) & 0x1){c[1] += " - ""BIOS shadowing is allowed""\r\n";}
			if((ch >> 13) & 0x1){c[1] += " - ""VL-VESA is supported""\r\n";}
			if((ch >> 14) & 0x1){c[1] += " - ""ESCD support is available""\r\n";}
			if((ch >> 15) & 0x1){c[1] += " - ""Boot from CD is supported""\r\n";}
			if((ch >> 16) & 0x1){c[1] += " - ""Selectable Boot is supported""\r\n";}
			if((ch >> 17) & 0x1){c[1] += " - ""BIOS ROM is socketed""\r\n";}
			if((ch >> 18) & 0x1){c[1] += " - ""Boot From PC Card (PCMCIA) is supported""\r\n";}
			if((ch >> 19) & 0x1){c[1] += " - ""EDD (Enhanced Disk Drive) Specification is supported""\r\n";}
			if((ch >> 20) & 0x1){c[1] += " - ""Int 13h - Japanese Floppy for NEC 9800 1.2mb (3.5\", 1k Bytes/Sector, 360 RPM) is supported""\r\n";}
			if((ch >> 21) & 0x1){c[1] += " - ""Int 13h - Japanese Floppy for Toshiba 1.2mb (3.5\", 360 RPM) is supported""\r\n";}
			if((ch >> 22) & 0x1){c[1] += " - ""Int 13h - 5.25\" / 360 KB Floppy Services are supported""\r\n";}
			if((ch >> 23) & 0x1){c[1] += " - ""Int 13h - 5.25\" /1.2MB Floppy Services are supported""\r\n";}
			if((ch >> 24) & 0x1){c[1] += " - ""Int 13h - 3.5\" / 720 KB Floppy Services are supported""\r\n";}
			if((ch >> 25) & 0x1){c[1] += " - ""Int 13h - 3.5\" / 2.88 MB Floppy Services are supported""\r\n";}
			if((ch >> 26) & 0x1){c[1] += " - ""Int 5h, Print Screen Service is supported""\r\n";}
			if((ch >> 27) & 0x1){c[1] += " - ""Int 9h, 8042 Keyboard services are supported""\r\n";}
			if((ch >> 28) & 0x1){c[1] += " - ""Int 14h, Serial Services are supported""\r\n";}
			if((ch >> 29) & 0x1){c[1] += " - ""Int 17h, Printer Services are supported""\r\n";}
			if((ch >> 30) & 0x1){c[1] += " - ""Int 10h, CGA/Mono Video Services are supported""\r\n";}
			if((ch >> 31) & 0x1){c[1] += " - ""NEC PC-98""\r\n";}
			if(p[1] > 0x12){
				c[1] += "BIOS Characteristics Extension Byte 1 : " + DmiStringBX(p[0x12], 2);
				ch = p[0x12];
				if((ch >>  0) & 0x1){c[1] += " - ""ACPI supported""\r\n";}
				if((ch >>  1) & 0x1){c[1] += " - ""USB Legacy is supported""\r\n";}
				if((ch >>  2) & 0x1){c[1] += " - ""AGP is supported""\r\n";}
				if((ch >>  3) & 0x1){c[1] += " - ""I2O boot is supported""\r\n";}
				if((ch >>  4) & 0x1){c[1] += " - ""LS-120 boot is supported""\r\n";}
				if((ch >>  5) & 0x1){c[1] += " - ""ATAPI ZIP Drive boot is supported""\r\n";}
				if((ch >>  6) & 0x1){c[1] += " - ""1394 boot is supported""\r\n";}
				if((ch >>  7) & 0x1){c[1] += " - ""Smart Battery supported""\r\n";}
			}

			if(p[1] > 0x13){
				c[1] += "BIOS Characteristics Extension Byte 2 : " + DmiStringBX(p[0x13], 2);
				ch = p[0x13];
				if((ch >>  0) & 0x1){c[1] += " - ""BIOS Boot Specification supported""\r\n";}
				if((ch >>  1) & 0x1){c[1] += " - ""Function key-initiated Network Service boot supported.""\r\n";}
				if((ch >>  2) & 0x1){c[1] += " - ""Enable Targeted Content Distribution.""\r\n";}
				if((ch >>  3) & 0x1){c[1] += " - ""Reserved""\r\n";}
				if((ch >>  4) & 0x1){c[1] += " - ""Reserved""\r\n";}
				if((ch >>  5) & 0x1){c[1] += " - ""Reserved""\r\n";}
				if((ch >>  6) & 0x1){c[1] += " - ""Reserved""\r\n";}
				if((ch >>  7) & 0x1){c[1] += " - ""Reserved""\r\n";}
			}
			if(flag[1]){m_Select.AddString(_T(" 00 BIOS Information"));flag[1] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 01 System Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 1:
			c[2] += PrintHeader(dmi);
			c[2] += "Manufacturer                   : " + DmiString(dmi, p[4]);
			c[2] += "Product Name                   : " + DmiString(dmi, p[5]);
			c[2] += "Version                        : " + DmiString(dmi, p[6]);
			c[2] += "Serial Number                  : " + DmiString(dmi, p[7]);
			
			// clarification of UUID format at SMBIOS 2.6
			c[2] += "UUID (Universal Unique ID)     : " + DmiStringBX(p[0x8], 1, 0) // time_low, DWORD
														+ DmiStringBX(p[0x9], 1, 0)
														+ DmiStringBX(p[0xA], 1, 0)
														+ DmiStringBX(p[0xB], 1, 0)
														+ "-"
														+ DmiStringBX(p[0xC], 1, 0) // time_mid, WORD
														+ DmiStringBX(p[0xD], 1, 0) 
														+ "-"
														+ DmiStringBX(p[0xE], 1, 0) // time_hi_and_version, WORD
														+ DmiStringBX(p[0xF], 1, 0)
														+ "-"
														+ DmiStringBX(p[0x10], 1, 0) // clock_seq_hi_and_reserved, BYTE
														+ DmiStringBX(p[0x11], 1, 0) // clock_seq_low
														+ "-"
														+ DmiStringBX(p[0x12], 1, 0) // Node 6BYTEs
														+ DmiStringBX(p[0x13], 1, 0)
														+ DmiStringBX(p[0x14], 1, 0)
														+ DmiStringBX(p[0x15], 1, 0)
														+ DmiStringBX(p[0x16], 1, 0)
														+ DmiStringBX(p[0x17], 1);
			c[2] += "Wake-up Type                   : ";
			switch (p[0x18])
			{
			case 0:	c[2] += "Reserved";				break;
			case 1:	c[2] += "Other";				break;
			case 2:	c[2] += "Unknown";				break;
			case 3:	c[2] += "APM Timer";			break;
			case 4:	c[2] += "Modem Ring";			break;
			case 5:	c[2] += "LAN Remote";			break;
			case 6:	c[2] += "Power Switch";			break;
			case 7:	c[2] += "PCI PME#";				break;
			case 8:	c[2] += "AC Power Restored";	break;
			default:c[2] += "Unknown";				break;
			}
			c[2] += "\r\n";
			if(flag[2]){m_Select.AddString(_T(" 01 System Information"));flag[2] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 02 Base Board Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 2:
			c[3] += PrintHeader(dmi);
			c[3] += "Manufacturer                   : " + DmiString(dmi, p[4]);
			c[3] += "Product                        : " + DmiString(dmi, p[5]);
			c[3] += "Version                        : " + DmiString(dmi, p[6]);
			c[3] += "Serial Number                  : " + DmiString(dmi, p[7]);
			if(p[2] > 8){
				c[3] += "Asset Tag                      : " + DmiString(dmi, p[8]);
			}
			if(p[1] > 9){
				c[3] += "Feature Flags                  : " + DmiStringBX(p[9], 2);
				ch = p[9];
				if((ch >>  0) & 0x1){c[1] += " - ""the board is a hosting board""\r\n";}
				if((ch >>  1) & 0x1){c[1] += " - ""the board requires at least one daughter board or auxiliary card to function properly""\r\n";}
				if((ch >>  2) & 0x1){c[1] += " - ""the board is removable""\r\n";}
				if((ch >>  3) & 0x1){c[1] += " - ""the board is replaceable""\r\n";}
				if((ch >>  4) & 0x1){c[1] += " - ""the board is hot swappable""\r\n";}
				if((ch >>  5) & 0x1){c[1] += " - ""Reserved""\r\n";}
				if((ch >>  6) & 0x1){c[1] += " - ""Reserved""\r\n";}
				if((ch >>  7) & 0x1){c[1] += " - ""Reserved""\r\n";}
			}
			if(p[1] > 0xA){		
				c[3] += "Location in Chassis            : " + DmiString(dmi, p[0xA]);
			}
			if(p[1] > 0xC){
				c[3] += "Chassis Handle                 : " + DmiStringWX(MAKEWORD(p[0xB], p[0xC]), 3);
			}
			if(p[1] > 0xD){
				c[3] += "Board Type                     : ";
				switch (p[0xD])
				{
				case 0x0:	c[3] += "Reserved";												break;
				case 0x1:	c[3] += "Unknown";												break;
				case 0x2:	c[3] += "Other";												break;
				case 0x3:	c[3] += "Server Blade";											break;
				case 0x4:	c[3] += "Connectivity Switch";									break;
				case 0x5:	c[3] += "System Management Module";								break;
				case 0x6:	c[3] += "Processor Module";										break;
				case 0x7:	c[3] += "I/O Module";											break;
				case 0x8:	c[3] += "Memory Module";										break;
				case 0x9:	c[3] += "Daughter board";										break;
				case 0xA:	c[3] += "Motherboard (includes processor, memory, and I/O)";	break;
				case 0xB:	c[3] += "Processor/Memory Module";								break;
				case 0xC:	c[3] += "Processor/IO Module";									break;
				case 0xD:	c[3] += "Interconnect Board";									break;
				default:	c[3] += "Unknown";												break;
				}
				c[3] += "\r\n"; 
			}
			if(p[1] > 0xE){
			c[3] += "Contained Object Handles       : " + DmiStringBX(p[0xE]);
			}
			if(p[1] > 0xF){
				for(i = 0; i < p[0xE]; i++){
					c[3] += " - " + DmiStringWX(MAKEWORD(p[0xF + i * 2], p[0xF + i * 2 + 1]), 2);
				}
			}
			if(flag[3]){m_Select.AddString(_T(" 02 Base Board Information"));flag[3] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 03 System Enclosure or Chassis
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 3:
			c[4] += PrintHeader(dmi);
			c[4] += "Manufacturer                   : " + DmiString(dmi, p[4]);
			c[4] += "Type                           : ";
			switch (p[0x5] & 0x7f){
			case 0x01:	c[4] += "Other";					break;
			case 0x02:	c[4] += "Unknown";					break;
			case 0x03:	c[4] += "Desktop";					break;
			case 0x04:	c[4] += "Low Profile Desktop";		break;
			case 0x05:	c[4] += "Pizza Box";				break;
			case 0x06:	c[4] += "Mini Tower";				break;
			case 0x07:	c[4] += "Tower";					break;
			case 0x08:	c[4] += "Portable";					break;
			case 0x09:	c[4] += "LapTop";					break;
			case 0x0A:	c[4] += "Notebook";					break;
			case 0x0B:	c[4] += "Hand Held";				break;
			case 0x0C:	c[4] += "Docking Station";			break;
			case 0x0D:	c[4] += "All in One";				break;
			case 0x0E:	c[4] += "Sub Notebook";				break;
			case 0x0F:	c[4] += "Space-saving";				break;
			case 0x10:	c[4] += "Lunch Box";				break;
			case 0x11:	c[4] += "Main Server Chassis";		break;
			case 0x12:	c[4] += "Expansion Chassis";		break;
			case 0x13:	c[4] += "SubChassis";				break;
			case 0x14:	c[4] += "Bus Expansion Chassis";	break;
			case 0x15:	c[4] += "Peripheral Chassis";		break;
			case 0x16:	c[4] += "RAID Chassis";				break;
			case 0x17:	c[4] += "Rack Mount Chassis";		break;
			case 0x18:	c[4] += "Sealed-case PC";			break;
			case 0x19:	c[4] += "Multi-system chassis";		break;
			case 0x1A:	c[4] += "CompactPCI";				break;
			case 0x1B:	c[4] += "AdvancedTCA";				break;
			case 0x1C:	c[4] += "Blade";					break;
			case 0x1D:	c[4] += "Blade Enclosure";			break;
			default:										break;
			}
			c[4] += "\r\n"; 
			c[4] += "Version                        : " + DmiString(dmi, p[6]);
			c[4] += "Serial Number                  : " + DmiString(dmi, p[7]);
			c[4] += "Asset Tag Number               : " + DmiString(dmi, p[8]);
			if(p[1] > 0xC){
				c[4] += "Boot-up State                  : ";
				switch (p[0x9])
				{
				case 0x01:	c[4] += "Other";			break;
				case 0x02:	c[4] += "Unknown";			break;
				case 0x03:	c[4] += "Safe";				break;
				case 0x04:	c[4] += "Warning";			break;
				case 0x05:	c[4] += "Critical";			break;
				case 0x06:	c[4] += "Non-recoverable";	break;
				}
				c[4] += "\r\n"; 
				c[4] += "Power Supply State             : ";
				switch (p[0xA])
				{
				case 0x01:	c[4] += "Other";			break;
				case 0x02:	c[4] += "Unknown";			break;
				case 0x03:	c[4] += "Safe";				break;
				case 0x04:	c[4] += "Warning";			break;
				case 0x05:	c[4] += "Critical";			break;
				case 0x06:	c[4] += "Non-recoverable";	break;
				}
				c[4] += "\r\n"; 
				c[4] += "Thermal State                  : ";
				switch (p[0xB])
				{
				case 0x01:	c[4] += "Other";			break;
				case 0x02:	c[4] += "Unknown";			break;
				case 0x03:	c[4] += "Safe";				break;
				case 0x04:	c[4] += "Warning";			break;
				case 0x05:	c[4] += "Critical";			break;
				case 0x06:	c[4] += "Non-recoverable";	break;
				}
				c[4] += "\r\n"; 
				c[4] += "Security Status                : ";
				switch (p[0xC])
				{
				case 0x01:	c[4] += "Other";									break;
				case 0x02:	c[4] += "Unknown";									break;
				case 0x03:	c[4] += "None";										break;
				case 0x04:	c[4] += "External interface locked out";			break;
				case 0x05:	c[4] += "External interface enabled";				break;
				}
				c[4] += "\r\n";
			}
			if(p[1] > 0x10){
				c[4] += "OEM-defined                    : " + DmiStringDX(MAKEDWORD(p[0xD], p[0xE], p[0xF], p[0x10]), 2);
			}
			if(p[1] > 0x11){
				c[4] += "Height                         : " + DmiStringBX(p[0x11], 0, 1);
				if(p[0x11] > 0){
					cstr.Format(_T("%.3f inches / %.3f cm"), p[0x11] * 1.75, p[0x11] * 4.445);
					c[4] += cstr;
				}
				c[4] += "\r\n";
			}
			if(p[1] > 0x12){
				c[4] += "Number of Power Cords          : " + DmiStringBX(p[0x12]);
			}
			// 0x13/0x14/0x15 are not supported!!
			if(flag[4]){m_Select.AddString(_T(" 03 System Enclosure or Chassis"));flag[4] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 04 Processor Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 4:
			c[5] += PrintHeader(dmi);
			c[5] += "Socket Designation             : " + DmiString(dmi, p[4]);
			c[5] += "Processor Type                 : ";
			switch (p[0x5])
			{
			case 0x01:	c[5] += "Other";				break;
			case 0x02:	c[5] += "Unknown";				break;
			case 0x03:	c[5] += "Central Processor";	break;
			case 0x04:	c[5] += "Math Processor";		break;
			case 0x05:	c[5] += "DSP Processor";		break;
			case 0x06:	c[5] += "Video Processor";		break;
			}
			c[5] += "\r\n";
			c[5] += "Processor Family               : ";
			
			if(p[0x1] > 0x28 && p[0x6] == 0xFE)
			{
				processorFamily = MAKEWORD(p[0x28], p[0x29]);
			}
			else
			{
				processorFamily = p[0x6];
			}

			switch (processorFamily)
			{
			case 0x01:	c[5] += "Other";																break;
			case 0x02:	c[5] += "Unknown";																break;
			case 0x03:	c[5] += "8086";																	break;
			case 0x04:	c[5] += "80286";																break;
			case 0x05:	c[5] += "Intel386. processor";													break;
			case 0x06:	c[5] += "Intel486. processor";													break;
			case 0x07:	c[5] += "8087";																	break;
			case 0x08:	c[5] += "80287";																break;
			case 0x09:	c[5] += "80387";																break;
			case 0x0A:	c[5] += "80487";																break;
			case 0x0B:	c[5] += "Pentium(R) processor Family";											break;
			case 0x0C:	c[5] += "Pentium(R) Pro processor";												break;
			case 0x0D:	c[5] += "Pentium(R) II processor";												break;
			case 0x0E:	c[5] += "Pentium(R) processor with MMX(TM) technology";							break;
			case 0x0F:	c[5] += "Celeron(TM) processor";												break;
			case 0x10:	c[5] += "Pentium(R) II Xeon(TM) processor";										break;
			case 0x11:	c[5] += "Pentium(R) III processor";												break;
			case 0x12:	c[5] += "M1 Family";															break;
			case 0x13:	c[5] += "M2 Family";															break;
			case 0x18:	c[5] += "AMD Duron(TM) Processor Family";										break;
			case 0x19:	c[5] += "K5 Family";															break;
			case 0x1A:	c[5] += "K6 Family";															break;
			case 0x1B:	c[5] += "K6-2";																	break;
			case 0x1C:	c[5] += "K6-3";																	break;
			case 0x1D:	c[5] += "AMD Athlon(TM) Processor Family";										break;
			case 0x1E:	c[5] += "AMD29000 Family";														break;
			case 0x1F:	c[5] += "K6-2+";																break;
			case 0x20:	c[5] += "Power PC Family";														break;
			case 0x21:	c[5] += "Power PC 601";															break;
			case 0x22:	c[5] += "Power PC 603";															break;
			case 0x23:	c[5] += "Power PC 603+";														break;
			case 0x24:	c[5] += "Power PC 604";															break;
			case 0x25:	c[5] += "Power PC 620";															break;
			case 0x26:	c[5] += "Power PC x704";														break;
			case 0x27:	c[5] += "Power PC 750";															break;
			case 0x30:	c[5] += "Alpha Family";															break;
			case 0x31:	c[5] += "Alpha 21064";															break;
			case 0x32:	c[5] += "Alpha 21066";															break;
			case 0x33:	c[5] += "Alpha 21164";															break;
			case 0x34:	c[5] += "Alpha 21164PC";														break;
			case 0x35:	c[5] += "Alpha 21164a";															break;
			case 0x36:	c[5] += "Alpha 21264";															break;
			case 0x37:	c[5] += "Alpha 21364";															break;
			case 0x40:	c[5] += "MIPS Family";															break;
			case 0x41:	c[5] += "MIPS R4000";															break;
			case 0x42:	c[5] += "MIPS R4200";															break;
			case 0x43:	c[5] += "MIPS R4400";															break;
			case 0x44:	c[5] += "MIPS R4600";															break;
			case 0x45:	c[5] += "MIPS R10000";															break;
			case 0x50:	c[5] += "SPARC Family";															break;
			case 0x51:	c[5] += "SuperSPARC";															break;
			case 0x52:	c[5] += "microSPARC II";														break;
			case 0x53:	c[5] += "microSPARC IIep";														break;
			case 0x54:	c[5] += "UltraSPARC";															break;
			case 0x55:	c[5] += "UltraSPARC II";														break;
			case 0x56:	c[5] += "UltraSPARC IIi";														break;
			case 0x57:	c[5] += "UltraSPARC III";														break;
			case 0x58:	c[5] += "UltraSPARC IIIi";														break;
			case 0x60:	c[5] += "68040 Family";															break;
			case 0x61:	c[5] += "68xxx";																break;
			case 0x62:	c[5] += "68000";																break;
			case 0x63:	c[5] += "68010";																break;
			case 0x64:	c[5] += "68020";																break;
			case 0x65:	c[5] += "68030";																break;
			case 0x70:	c[5] += "Hobbit Family";														break;
			case 0x78:	c[5] += "Crusoe(TM) TM5000 Family";												break;
			case 0x79:	c[5] += "Crusoe(TM) TM3000 Family";												break;
			case 0x7A:	c[5] += "Efficeon(TM) TM8000 Family";											break;
			case 0x80:	c[5] += "Weitek";																break;
			case 0x82:	c[5] += "Itanium(TM)";															break;
			case 0x83:	c[5] += "AMD Athlon(TM) 64 Processor Family";									break;
			case 0x84:	c[5] += "AMD Opteron(TM) Processor Family";										break;
			case 0x85:	c[5] += "AMD Sempron(TM) Processor Family";										break;
			case 0x86:	c[5] += "AMD Turion(TM) 64 Mobile Technology";									break;
			case 0x87:	c[5] += "Dual-Core AMD Opteron(TM) Processor Family";							break;
			case 0x88:	c[5] += "AMD Athlon(TM) 64 X2 Dual-Core Processor Family";						break;
			case 0x89:	c[5] += "AMD Turion(TM) 64 X2 Mobile Technology";								break;
			case 0x90:	c[5] += "PA-RISC Family";														break;
			case 0x91:	c[5] += "PA-RISC 8500";															break;
			case 0x92:	c[5] += "PA-RISC 8000";															break;
			case 0x93:	c[5] += "PA-RISC 7300LC";														break;
			case 0x94:	c[5] += "PA-RISC 7200";															break;
			case 0x95:	c[5] += "PA-RISC 7100LC";														break;
			case 0x96:	c[5] += "PA-RISC 7100";															break;
			case 0xA0:	c[5] += "V30 Family";															break;
			case 0xB0:	c[5] += "Pentium(R) III Xeon(TM) processor";									break;
			case 0xB1:	c[5] += "Pentium(R) III Processor with Intel (R) SpeedStep(TM) Technology";		break;
			case 0xB2:	c[5] += "Pentium(R) 4 Processor";												break;
			case 0xB3:	c[5] += "Intel(R) Xeon(TM)";													break;
			case 0xB4:	c[5] += "AS400 Family";															break;
			case 0xB5:	c[5] += "Intel(R) Xeon(TM) processor MP";										break;
			case 0xB6:	c[5] += "AMD Athlon(TM) XP Processor Family";									break;
			case 0xB7:	c[5] += "AMD Athlon(TM) MP Processor Family";									break;
			case 0xB8:	c[5] += "Intel(R) Itanium(R) 2 processor";										break;
			case 0xB9:	c[5] += "Intel(R) Pentium(R) M processor";										break;
			case 0xBA:	c[5] += "Intel(R) Celeron(R) D processor";										break;
			case 0xBB:	c[5] += "Intel(R) Pentium(R) D processor";										break;
			case 0xBC:	c[5] += "Intel(R) Pentium(R) Processor Extreme Edition";						break;
			case 0xBD:	c[5] += "Intel(R) Core(TM) Solo Processor";										break;
			case 0xBE:	c[5] += "Intel(R) Core(TM) 2 / AMD K7";											break;
			case 0xBF:	c[5] += "Intel(R) Core(TM) 2 Duo Processor";									break;
			case 0xC8:	c[5] += "IBM390 Family";														break;
			case 0xC9:	c[5] += "G4";																	break;
			case 0xCA:	c[5] += "G5";																	break;
			case 0xCB:	c[5] += "ESA/390 G6";															break;
			case 0xCC:	c[5] += "z/Architectur base";													break;
			case 0xD2:	c[5] += "VIA C7(TM)-M Processor Family";										break;
			case 0xD3:	c[5] += "VIA C7(TM)-D Processor Family";										break;
			case 0xD4:	c[5] += "VIA C7(TM) Processor Family";											break;
			case 0xD5:	c[5] += "VIA Eden(TM) Processor Family";										break;
			case 0xFA:	c[5] += "i860";																	break;
			case 0xFB:	c[5] += "i960";																	break;

			// for Processor Family 2 field //
			case 0x104:	c[5] += "SH-3";																	break;
			case 0x105:	c[5] += "SH-4";																	break;
			case 0x118:	c[5] += "ARM";																	break;
			case 0x119:	c[5] += "StrongARM";															break;
			case 0x12C:	c[5] += "6x86";																	break;
			case 0x12D:	c[5] += "MediaGX";																break;
			case 0x12E:	c[5] += "MII";																	break;
			case 0x140:	c[5] += "WinChip";																break;
			case 0x15E:	c[5] += "DSP";																	break;
			case 0x1F4:	c[5] += "Video Processor";														break;

			}
			c[5] += "\r\n";
			c[5] += "Processor Manufacturer         : " + DmiString(dmi, p[7]);
			c[5] += "Processor ID                   : " + DmiStringBX(p[0xF], 1, 0)
														+ DmiStringBX(p[0xE], 1, 0)
														+ DmiStringBX(p[0xD], 1, 0)
														+ DmiStringBX(p[0xC], 1, 0)
														+ DmiStringBX(p[0xB], 1, 0)
														+ DmiStringBX(p[0xA], 1, 0)
														+ DmiStringBX(p[0x9], 1, 0)
														+ DmiStringBX(p[0x8], 1);
			c[5] += "Processor Version              : " + DmiString(dmi, p[0x10]);
			c[5] += "Voltage                        : ";
			if((p[0x11] >> 7) & 0x1){
				if((p[0x11] & 0x7F) != 0){
					cstr.Format(_T("%.1fV"), (p[0x11] & 0x7F) / 10.0);
					c[5] += cstr;
				}else{
					c[5] += "Unknown";
				}
			}else{ // legacy
				if((p[0x11] >> 0) & 0x1){c[5] += "5.0V ";}
				if((p[0x11] >> 1) & 0x1){c[5] += "3.3V ";}
				if((p[0x11] >> 2) & 0x1){c[5] += "2.9V ";}
			}
			c[5] += "\r\n";
			c[5] += "External Clock                 : " + DmiStringW(MAKEWORD(p[0x12], p[0x13]), 0) + " MHz\r\n";
			c[5] += "Max Speed                      : " + DmiStringW(MAKEWORD(p[0x14], p[0x15]), 0) + " MHz\r\n";
			c[5] += "Current Speed                  : " + DmiStringW(MAKEWORD(p[0x16], p[0x17]), 0) + " MHz\r\n";
			c[5] += "Status - CPU Socket Populated  : ";
			if((p[0x18] >> 6) & 0x1){
				c[5] += "CPU Socket Populated";
			}else{
				c[5] += "CPU Socket Unpopulated";
			}
			c[5] += "\r\n";
			c[5] += "Status - CPU Status            : ";
			switch( p[0x18] & 0x7 ){
			case 0: c[5] += "Unknown";									break;
			case 1: c[5] += "CPU Enabled";								break;
			case 2: c[5] += "CPU Disabled by User via BIOS Setup";		break;
			case 3: c[5] += "CPU Disabled By BIOS (POST Error)";		break;
			case 4: c[5] += "CPU is Idle, waiting to be enabled.";		break;
			case 5: c[5] += "Reserved";									break;
			case 6: c[5] += "Reserved";									break;
			case 7: c[5] += "Other";									break;
			}
			c[5] += "\r\n";
			c[5] += "Processor Upgrade              : ";
			switch( p[0x19] ){
			case 0x01: c[5] += "Other";						break;
			case 0x02: c[5] += "Unknown";					break;
			case 0x03: c[5] += "Daughter Board";			break;
			case 0x04: c[5] += "ZIF Socket";				break;
			case 0x05: c[5] += "Replaceable Piggy Back";	break;
			case 0x06: c[5] += "None";						break;
			case 0x07: c[5] += "LIF Socket";				break;
			case 0x08: c[5] += "Slot 1";					break;
			case 0x09: c[5] += "Slot 2";					break;
			case 0x0A: c[5] += "370-pin socket";			break;
			case 0x0B: c[5] += "Slot A";					break;
			case 0x0C: c[5] += "Slot M";					break;
			case 0x0D: c[5] += "Socket 423";				break;
			case 0x0E: c[5] += "Socket A (Socket 462)";		break;
			case 0x0F: c[5] += "Socket 478";				break;
			case 0x10: c[5] += "Socket 754";				break;
			case 0x11: c[5] += "Socket 940";				break;
			case 0x12: c[5] += "Socket 939";				break;
			case 0x13: c[5] += "Socket mPGA604";			break;
			case 0x14: c[5] += "Socket LGA771";				break;
			case 0x15: c[5] += "Socket LGA775";				break;
			case 0x16: c[5] += "Socket S1";					break;
			case 0x17: c[5] += "Socket AM2";				break;
			case 0x18: c[5] += "Socket F (1207)";			break;
			default  : 
				CString cstr;
				cstr.Format(_T("[0x%02X]"), p[0x19]);
				c[5] += cstr;
				break;
			}

			c[5] += "\r\n";
			c[5] += "L1 Cache Handle                : " + DmiStringWX(MAKEWORD(p[0x1A], p[0x1B]), 3);
			c[5] += "L2 Cache Handle                : " + DmiStringWX(MAKEWORD(p[0x1C], p[0x1D]), 3);
			c[5] += "L3 Cache Handle                : " + DmiStringWX(MAKEWORD(p[0x1E], p[0x1F]), 3);

			if(p[0x1] > 0x20){
				c[5] += "Serial Number                  : " + DmiString(dmi, p[0x20]);
			}
			if(p[0x1] > 0x21){
				c[5] += "Asset Tag                      : " + DmiString(dmi, p[0x21]);
			}
			if(p[0x1] > 0x22){
				c[5] += "Part Number                    : " + DmiString(dmi, p[0x22]);
			}
			if(p[0x1] > 0x23){
				c[5] += "Core Count                     : " + DmiStringB(p[0x23]);
			}
			if(p[0x1] > 0x24){
				c[5] += "Core Enabled                   : " + DmiStringB(p[0x24]);
			}
			if(p[0x1] > 0x25){
				c[5] += "Thread Count                   : " + DmiStringB(p[0x25]);
			}
			if(p[0x1] > 0x26){
				c[5] += "Processor Characteristics      : " + DmiStringWX(MAKEWORD(p[0x26], p[0x27]), 2);
				WORD ch = MAKEWORD(p[0x26], p[0x27]);
				if((ch >>  0) & 0x1){c[5] += " - ""Reserved""\r\n";}
				if((ch >>  1) & 0x1){c[5] += " - ""Unknown""\r\n";}
				if((ch >>  2) & 0x1){c[5] += " - ""64-bit Capable""\r\n";}
				if((ch >>  3) & 0x1){c[5] += " - ""Reserved""\r\n";}
				if((ch >>  4) & 0x1){c[5] += " - ""Reserved""\r\n";}
				if((ch >>  5) & 0x1){c[5] += " - ""Reserved""\r\n";}
				if((ch >>  6) & 0x1){c[5] += " - ""Reserved""\r\n";}
				if((ch >>  7) & 0x1){c[5] += " - ""Reserved""\r\n";}
			}

			if(flag[5]){m_Select.AddString(_T(" 04 Processor Information"));flag[5] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 05 Memory Controller Information (Obsolete)
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 5:
			c[6] += PrintHeader(dmi);
			c[6] += "Error Detecting Method         : ";
			switch( p[0x4] ){
			case 0x01: c[6] += "Other";			break;
			case 0x02: c[6] += "Unknown";		break;
			case 0x03: c[6] += "None";			break;
			case 0x04: c[6] += "8-bit Parity";	break;
			case 0x05: c[6] += "32-bit ECC";	break;
			case 0x06: c[6] += "64-bit ECC";	break;
			case 0x07: c[6] += "128-bit ECC";	break;
			case 0x08: c[6] += "CRC";			break;
			}
			c[6] += "\r\n";
	
			c[6] += "Error Correcting Capability    : " + DmiStringBX(p[0x5], 2);
			ch = p[0x5];
			if((ch >>  0) & 0x1){c[6] += " - ""Other""\r\n";}
			if((ch >>  1) & 0x1){c[6] += " - ""Unknown""\r\n";}
			if((ch >>  2) & 0x1){c[6] += " - ""None""\r\n";}
			if((ch >>  3) & 0x1){c[6] += " - ""Single Bit Error Correcting""\r\n";}
			if((ch >>  4) & 0x1){c[6] += " - ""Double Bit Error Correcting""\r\n";}
			if((ch >>  5) & 0x1){c[6] += " - ""Error Scrubbing""\r\n";}

			c[6] += "Supported Interleave           : ";
			switch( p[0x6] ){
			case 0x01: c[6] += "Other";						break;
			case 0x02: c[6] += "Unknown";					break;
			case 0x03: c[6] += "One Way Interleave";		break;
			case 0x04: c[6] += "Two Way Interleave";		break;
			case 0x05: c[6] += "Four Way Interleave";		break;
			case 0x06: c[6] += "Eight Way Interleave";		break;
			case 0x07: c[6] += "Sixteen Way Interleave";	break;
			}
			c[6] += "\r\n";

			c[6] += "Current Interleave             : ";
			switch( p[0x7] ){
			case 0x01: c[6] += "Other";						break;
			case 0x02: c[6] += "Unknown";					break;
			case 0x03: c[6] += "One Way Interleave";		break;
			case 0x04: c[6] += "Two Way Interleave";		break;
			case 0x05: c[6] += "Four Way Interleave";		break;
			case 0x06: c[6] += "Eight Way Interleave";		break;
			case 0x07: c[6] += "Sixteen Way Interleave";	break;
			}
			c[6] += "\r\n";

			cstr.Format(_T("%d MB\r\n"), 1 << p[0x8]);
			c[6] += "Maximum Memory Module Size     : " + cstr;
			c[6] += "Supported Speeds               : " + DmiStringWX(MAKEWORD(p[0x9], p[0xA]), 2);
			ch = MAKEWORD(p[0x9], p[0xA]);
			if((ch >>  0) & 0x1){c[6] += " - ""Other""\r\n";}
			if((ch >>  1) & 0x1){c[6] += " - ""Unknown""\r\n";}
			if((ch >>  2) & 0x1){c[6] += " - ""70ns""\r\n";}
			if((ch >>  3) & 0x1){c[6] += " - ""60ns""\r\n";}
			if((ch >>  4) & 0x1){c[6] += " - ""50ns""\r\n";}

			c[6] += "Supported Memory Types         : " + DmiStringWX(MAKEWORD(p[0xB], p[0xC]), 2);
			ch = MAKEWORD(p[0xB], p[0xC]);
			if((ch >>  0) & 0x1){c[6] += " - ""Other""\r\n";}
			if((ch >>  1) & 0x1){c[6] += " - ""Unknown""\r\n";}
			if((ch >>  2) & 0x1){c[6] += " - ""Standard""\r\n";}
			if((ch >>  3) & 0x1){c[6] += " - ""Fast Page Mode""\r\n";}
			if((ch >>  4) & 0x1){c[6] += " - ""EDO""\r\n";}
			if((ch >>  5) & 0x1){c[6] += " - ""Parity""\r\n";}
			if((ch >>  6) & 0x1){c[6] += " - ""ECC""\r\n";}
			if((ch >>  7) & 0x1){c[6] += " - ""SIMM""\r\n";}
			if((ch >>  8) & 0x1){c[6] += " - ""DIMM""\r\n";}
			if((ch >>  9) & 0x1){c[6] += " - ""Burst EDO""\r\n";}
			if((ch >> 10) & 0x1){c[6] += " - ""SDRAM""\r\n";}

			c[6] += "Memory Module Voltage          : " + DmiStringBX(p[0xD], 2);
			ch = p[0xD];
			if((ch >>  0) & 0x1){c[6] += " - ""5.0V""\r\n";}
			if((ch >>  1) & 0x1){c[6] += " - ""3.3V""\r\n";}
			if((ch >>  2) & 0x1){c[6] += " - ""2.9V""\r\n";}

			c[6] += "Number of Associated Memory Slots     : " + DmiStringB(p[0xE], 2);
			for(i = 0; i < p[0xE]; i++)
			{
			c[6] += "Memory Module Configuration Handles   : " + DmiStringWX(MAKEWORD(p[0xF + i*2], p[0xF + i*2 + 1]), 3);
			}
			c[6] += "Enabled Error Correcting Capabilities : " + DmiStringBX(p[0xF + i*2], 2);
			ch = p[0xF + i*2];
			if((ch >>  0) & 0x1){c[6] += " - ""Other""\r\n";}
			if((ch >>  1) & 0x1){c[6] += " - ""Unknown""\r\n";}
			if((ch >>  2) & 0x1){c[6] += " - ""None""\r\n";}
			if((ch >>  3) & 0x1){c[6] += " - ""Single Bit Error Correcting""\r\n";}
			if((ch >>  4) & 0x1){c[6] += " - ""Double Bit Error Correcting""\r\n";}
			if((ch >>  5) & 0x1){c[6] += " - ""Error Scrubbing""\r\n";}

			if(flag[6]){m_Select.AddString(_T(" 05 Memory Controller Information"));flag[6] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 06 Memory Module Information (Obsolete)
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 6:
			c[7] += PrintHeader(dmi);
			c[7] += "Socket Designation             : " + DmiString(dmi, p[0x4]);
			c[7] += "Bank Connections               : ";
			if((p[0x5] & 0xF) != 0xF && (p[0x5] >> 4) != 0xF){
				cstr.Format(_T("%d & %d"), p[0x5] >> 4, p[0x5] & 0xF);
				c[7] += cstr;
			}
			if((p[0x5] & 0xF) != 0xF && (p[0x5] >> 4) == 0xF){
				cstr.Format(_T("%d"), p[0x5] & 0xF);
				c[7] += cstr;
			}
			if((p[0x5] & 0xF) == 0xF && (p[0x5] >> 4) != 0xF){
				cstr.Format(_T("%d"), p[0x5] >> 4);
				c[7] += cstr;
			}
			if((p[0x5] & 0xF) == 0xF && (p[0x5] >> 4) == 0xF){
				c[7] += "";
			}
			c[7] += "\r\n";
			
			c[7] += "Current Speed                  : ";
			if(p[0x6] != 0){
				cstr.Format(_T("%d ns"), p[6]);
				c[7] += cstr;
			}else{
				c[7] += "Unknown";
			}
			c[7] += "\r\n";
			c[7] += "Current Memory Types           : " + DmiStringWX(MAKEWORD(p[0x7], p[0x8]), 2);
			ch = MAKEWORD(p[0x7], p[0x8]);
			if((ch >>  0) & 0x1){c[7] += " - ""Other""\r\n";}
			if((ch >>  1) & 0x1){c[7] += " - ""Unknown""\r\n";}
			if((ch >>  2) & 0x1){c[7] += " - ""Standard""\r\n";}
			if((ch >>  3) & 0x1){c[7] += " - ""Fast Page Mode""\r\n";}
			if((ch >>  4) & 0x1){c[7] += " - ""EDO""\r\n";}
			if((ch >>  5) & 0x1){c[7] += " - ""Parity""\r\n";}
			if((ch >>  6) & 0x1){c[7] += " - ""ECC""\r\n";}
			if((ch >>  7) & 0x1){c[7] += " - ""SIMM""\r\n";}
			if((ch >>  8) & 0x1){c[7] += " - ""DIMM""\r\n";}
			if((ch >>  9) & 0x1){c[7] += " - ""Burst EDO""\r\n";}
			if((ch >> 10) & 0x1){c[7] += " - ""SDRAM""\r\n";}

			c[7] += "Installed Size                 : ";
			if((p[0x9] & 0x7F) == 0x7D){c[7] += "Not determinable (Installed Size only)";}
			else if((p[0x9] & 0x7F) == 0x7D){c[7] += "Module is installed, but no memory has been enabled";}
			else if((p[0x9] & 0x7F) == 0x7F){c[7] += "Not installed";}
			else {
				cstr.Format(_T("%d MB"), 1 << (p[0x9] & 0x7F) );
				c[7] += cstr;
				if((p[0x9] >> 7) & 0x1){
					c[7] += " [double-bank]";
				}else{
					c[7] += " [single-bank]";
				}
			}
			c[7] += "\r\n";
			if(flag[7]){m_Select.AddString(_T(" 06 Memory Module Information"));flag[7] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 07 Cache Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 7:
			c[8] += PrintHeader(dmi);
			c[8] += "Socket Designation             : " + DmiString(dmi, p[0x4]);
			c[8] += "Cache Configuration            : " + DmiStringDX(MAKEWORD(p[0x5], p[0x06]), 2);
			word = MAKEWORD(p[0x5], p[0x06]);
			c[8] += " + Operational Mode            : ";
			switch( (word >> 8) & 0x3 ){
			case 0: c[8] += "Write Through";				break;
			case 1: c[8] += "Write Back";					break;
			case 2: c[8] += "Varies with Memory Address";	break;
			case 3: c[8] += "Unknown";						break;
			}
			c[8] += "\r\n";
			c[8] += " + State                       : ";
			if((word >> 7) & 0x1){
				c[8] += "Enable";
			}else{
				c[8] += "Disable";
			}
			c[8] += "\r\n";
			c[8] += " + Location                    : ";
			switch( (word >> 5) & 0x3 ){
			case 0: c[8] += "Internal";	break;
			case 1: c[8] += "External";	break;
			case 2: c[8] += "Reserved";	break;
			case 3: c[8] += "Unknown";	break;
			}
			c[8] += "\r\n";
			c[8] += " + Cache Socketed              : ";
			if((word >> 3) & 0x1){
				c[8] += "Socketed";
			}else{
				c[8] += "Not Socketed";
			}
			c[8] += "\r\n";
			c[8] += " + Cache Level                 : ";
			cstr.Format(_T("L%d"), (word & 0x7) + 1);
			c[8] += cstr;
			c[8] += "\r\n";

			c[8] += "Maximum Cache Size             : ";
			word = MAKEWORD(p[0x7], p[0x8]);
			if((word >> 15) & 0x1){
				cstr.Format(_T("%d KB"), (word & 0x7FFF) * 64);
				c[8] += cstr;
			}else{
				cstr.Format(_T("%d KB"), (word & 0x7FFF) * 1);
				c[8] += cstr;
			}
			c[8] += "\r\n";
			c[8] += "Installed Size                 : ";
			word = MAKEWORD(p[0x9], p[0xA]);
			if((word >> 15) & 0x1){
				cstr.Format(_T("%d KB"), (word & 0x7FFF) * 64);
				c[8] += cstr;
			}else{
				cstr.Format(_T("%d KB"), (word & 0x7FFF) * 1);
				c[8] += cstr;
			}
			c[8] += "\r\n";

			c[8] += "Supported SRAM Type            : " + DmiStringWX(MAKEWORD(p[0xB], p[0xC]), 2);
			ch = MAKEWORD(p[0xB], p[0xC]);
			if((ch >>  0) & 0x1){c[8] += " - ""Other""\r\n";}
			if((ch >>  1) & 0x1){c[8] += " - ""Unknown""\r\n";}
			if((ch >>  2) & 0x1){c[8] += " - ""Non-Burst""\r\n";}
			if((ch >>  3) & 0x1){c[8] += " - ""Burst""\r\n";}
			if((ch >>  4) & 0x1){c[8] += " - ""Pipeline Burst""\r\n";}
			if((ch >>  5) & 0x1){c[8] += " - ""Synchronous""\r\n";}
			if((ch >>  6) & 0x1){c[8] += " - ""Asynchronous""\r\n";}

			c[8] += "Current SRAM Type              : " + DmiStringWX(MAKEWORD(p[0xD], p[0xE]), 2);
			ch = MAKEWORD(p[0xD], p[0xE]);
			if((ch >>  0) & 0x1){c[8] += " - ""Other""\r\n";}
			if((ch >>  1) & 0x1){c[8] += " - ""Unknown""\r\n";}
			if((ch >>  2) & 0x1){c[8] += " - ""Non-Burst""\r\n";}
			if((ch >>  3) & 0x1){c[8] += " - ""Burst""\r\n";}
			if((ch >>  4) & 0x1){c[8] += " - ""Pipeline Burst""\r\n";}
			if((ch >>  5) & 0x1){c[8] += " - ""Synchronous""\r\n";}
			if((ch >>  6) & 0x1){c[8] += " - ""Asynchronous""\r\n";}
			
			c[8] += "Cache Speed                    : ";
			if(p[0xF] == 0){
				c[8] += "Unknown";
			}else{
				cstr.Format(_T("%d ns"), p[0xF]);
				c[8] += cstr;
			}
			c[8] += "\r\n";

			c[8] += "Error Correction Type          : ";
			switch( p[0x10] ){
			case 1: c[8] += "Other";			break;
			case 2: c[8] += "Unknown";			break;
			case 3: c[8] += "None";				break;
			case 4: c[8] += "Parity";			break;
			case 5: c[8] += "Single-bit ECC";	break;
			case 6: c[8] += "Multi-bit ECC";	break;
			}
			c[8] += "\r\n";

			c[8] += "System Cache Type              : ";
			switch( p[0x11] ){
			case 1: c[8] += "Other";			break;
			case 2: c[8] += "Unknown";			break;
			case 3: c[8] += "Instruction";		break;
			case 4: c[8] += "Data";				break;
			case 5: c[8] += "Unified";			break;
			}
			c[8] += "\r\n";

			c[8] += "Associativity                  : ";
			switch( p[0x12] ){
			case 1: c[8] += "Other";					break;
			case 2: c[8] += "Unknown";					break;
			case 3: c[8] += "Direct Mapped";			break;
			case 4: c[8] += "2-way Set-Associative";	break;
			case 5: c[8] += "4-way Set-Associative";	break;
			case 6: c[8] += "Fully Associative";		break;
			case 7: c[8] += "8-way Set-Associative";	break;
			case 8: c[8] += "16-way Set-Associative";	break;
			}
			c[8] += "\r\n";
			if(flag[8]){m_Select.AddString(_T(" 07 Cache Information"));flag[8] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 08 Port Connector Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 8:
			c[9] += PrintHeader(dmi);
			c[9] += "Internal Reference Designator  : " + DmiString(dmi, p[4]);
			c[9] += "Internal Connector Type        : ";
			switch( p[0x5] ){
			case 0x00: c[9] += "None";								break;
			case 0x01: c[9] += "Centronics";						break;
			case 0x02: c[9] += "Mini Centronics";					break;
			case 0x03: c[9] += "Proprietary";						break;
			case 0x04: c[9] += "DB-25 pin male";					break;
			case 0x05: c[9] += "DB-25 pin female";					break;
			case 0x06: c[9] += "DB-15 pin male";					break;
			case 0x07: c[9] += "DB-15 pin female";					break;
			case 0x08: c[9] += "DB-9 pin male";						break;
			case 0x09: c[9] += "DB-9 pin female";					break;
			case 0x0A: c[9] += "RJ-11";								break;
			case 0x0B: c[9] += "RJ-45";								break;
			case 0x0C: c[9] += "50 Pin MiniSCSI";					break;
			case 0x0D: c[9] += "Mini-DIN";							break;
			case 0x0E: c[9] += "Micro-DIN";							break;
			case 0x0F: c[9] += "PS/2";								break;
			case 0x10: c[9] += "Infrared";							break;
			case 0x11: c[9] += "HP-HIL";							break;
			case 0x12: c[9] += "Access Bus (USB)";					break;
			case 0x13: c[9] += "SSA SCSI";							break;
			case 0x14: c[9] += "Circular DIN-8 male";				break;
			case 0x15: c[9] += "Circular DIN-8 female";				break;
			case 0x16: c[9] += "On Board IDE";						break;
			case 0x17: c[9] += "On Board Floppy";					break;
			case 0x18: c[9] += "9 Pin Dual Inline (pin 10 cut)";	break;
			case 0x19: c[9] += "25 Pin Dual Inline (pin 26 cut)";	break;
			case 0x1A: c[9] += "50 Pin Dual Inline";				break;
			case 0x1B: c[9] += "68 Pin Dual Inline";				break;
			case 0x1C: c[9] += "On Board Sound Input from CD-ROM";	break;
			case 0x1D: c[9] += "Mini-Centronics Type-14";			break;
			case 0x1E: c[9] += "Mini-Centronics Type-26";			break;
			case 0x1F: c[9] += "Mini-jack (headphones)";			break;
			case 0x20: c[9] += "BNC";								break;
			case 0x21: c[9] += "1394";								break;
			case 0xA0: c[9] += "PC-98";								break;
			case 0xA1: c[9] += "PC-98Hireso";						break;
			case 0xA2: c[9] += "PC-H98";							break;
			case 0xA3: c[9] += "PC-98Note";							break;
			case 0xA4: c[9] += "PC-98Full";							break;
			case 0xFF: c[9] += "Other";								break;
			}
			c[9] += "\r\n";
			c[9] += "External Reference Designator  : " + DmiString(dmi, p[6]);
			c[9] += "External Connector Type        : ";
			switch( p[0x7] ){
			case 0x00: c[9] += "None";								break;
			case 0x01: c[9] += "Centronics";						break;
			case 0x02: c[9] += "Mini Centronics";					break;
			case 0x03: c[9] += "Proprietary";						break;
			case 0x04: c[9] += "DB-25 pin male";					break;
			case 0x05: c[9] += "DB-25 pin female";					break;
			case 0x06: c[9] += "DB-15 pin male";					break;
			case 0x07: c[9] += "DB-15 pin female";					break;
			case 0x08: c[9] += "DB-9 pin male";						break;
			case 0x09: c[9] += "DB-9 pin female";					break;
			case 0x0A: c[9] += "RJ-11";								break;
			case 0x0B: c[9] += "RJ-45";								break;
			case 0x0C: c[9] += "50 Pin MiniSCSI";					break;
			case 0x0D: c[9] += "Mini-DIN";							break;
			case 0x0E: c[9] += "Micro-DIN";							break;
			case 0x0F: c[9] += "PS/2";								break;
			case 0x10: c[9] += "Infrared";							break;
			case 0x11: c[9] += "HP-HIL";							break;
			case 0x12: c[9] += "Access Bus (USB)";					break;
			case 0x13: c[9] += "SSA SCSI";							break;
			case 0x14: c[9] += "Circular DIN-8 male";				break;
			case 0x15: c[9] += "Circular DIN-8 female";				break;
			case 0x16: c[9] += "On Board IDE";						break;
			case 0x17: c[9] += "On Board Floppy";					break;
			case 0x18: c[9] += "9 Pin Dual Inline (pin 10 cut)";	break;
			case 0x19: c[9] += "25 Pin Dual Inline (pin 26 cut)";	break;
			case 0x1A: c[9] += "50 Pin Dual Inline";				break;
			case 0x1B: c[9] += "68 Pin Dual Inline";				break;
			case 0x1C: c[9] += "On Board Sound Input from CD-ROM";	break;
			case 0x1D: c[9] += "Mini-Centronics Type-14";			break;
			case 0x1E: c[9] += "Mini-Centronics Type-26";			break;
			case 0x1F: c[9] += "Mini-jack (headphones)";			break;
			case 0x20: c[9] += "BNC";								break;
			case 0x21: c[9] += "1394";								break;
			case 0xA0: c[9] += "PC-98";								break;
			case 0xA1: c[9] += "PC-98Hireso";						break;
			case 0xA2: c[9] += "PC-H98";							break;
			case 0xA3: c[9] += "PC-98Note";							break;
			case 0xA4: c[9] += "PC-98Full";							break;
			case 0xFF: c[9] += "Other";								break;
			}
			c[9] += "\r\n";
			c[9] += "Port Type                      : ";
			switch( p[0x8] ){
			case 0x00: c[9] += "None";								break;
			case 0x01: c[9] += "Parallel Port XT/AT Compatible";	break;
			case 0x02: c[9] += "Parallel Port PS/2";				break;
			case 0x03: c[9] += "Parallel Port ECP";					break;
			case 0x04: c[9] += "Parallel Port EPP";					break;
			case 0x05: c[9] += "Parallel Port ECP/EPP";				break;
			case 0x06: c[9] += "Serial Port XT/AT Compatible";		break;
			case 0x07: c[9] += "Serial Port 16450 Compatible";		break;
			case 0x08: c[9] += "Serial Port 16550 Compatible";		break;
			case 0x09: c[9] += "Serial Port 16550A Compatible";		break;
			case 0x0A: c[9] += "SCSI Port";							break;
			case 0x0B: c[9] += "MIDI Port";							break;
			case 0x0C: c[9] += "Joy Stick Port";					break;
			case 0x0D: c[9] += "Keyboard Port";						break;
			case 0x0E: c[9] += "Mouse Port";						break;
			case 0x0F: c[9] += "SSA SCSI";							break;
			case 0x10: c[9] += "USB";								break;
			case 0x11: c[9] += "FireWire (IEEE P1394)";				break;
			case 0x12: c[9] += "PCMCIA Type II";					break;
			case 0x13: c[9] += "PCMCIA Type II";					break;
			case 0x14: c[9] += "PCMCIA Type III";					break;
			case 0x15: c[9] += "Cardbus";							break;
			case 0x16: c[9] += "Access Bus Port";					break;
			case 0x17: c[9] += "SCSI II";							break;
			case 0x18: c[9] += "SCSI Wide";							break;
			case 0x19: c[9] += "PC-98";								break;
			case 0x1A: c[9] += "PC-98-Hireso";						break;
			case 0x1B: c[9] += "PC-H98";							break;
			case 0x1C: c[9] += "Video Port";						break;
			case 0x1D: c[9] += "Audio Port";						break;
			case 0x1E: c[9] += "Modem Port";						break;
			case 0x1F: c[9] += "Network Port";						break;
			case 0x20: c[9] += "SATA";								break;
			case 0x21: c[9] += "SAS";								break;
			case 0xA0: c[9] += "8251 Compatible";					break;
			case 0xA1: c[9] += "8251 FIFO Compatible";				break;
			case 0xFF: c[9] += "Other";								break;
			}
			c[9] += "\r\n";
			if(flag[9]){m_Select.AddString(_T(" 08 Port Connector Information"));flag[9] = FALSE;}
			break;		
///////////////////////////////////////////////////////////////////////////////////////////////////
// 09 System Slots
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 9:
			c[10] += PrintHeader(dmi);
			c[10] += "Slot Designation               : " + DmiString(dmi, p[4]);
			c[10] += "Slot Type                      : ";
			switch( p[0x5] ){
			case 0x01: c[10] += "Other";						break;
			case 0x02: c[10] += "Unknown";						break;
			case 0x03: c[10] += "ISA";							break;
			case 0x04: c[10] += "MCA";							break;
			case 0x05: c[10] += "EISA";							break;
			case 0x06: c[10] += "PCI";							break;
			case 0x07: c[10] += "PC Card (PCMCIA)";				break;
			case 0x08: c[10] += "VL-VESA";						break;
			case 0x09: c[10] += "Proprietary";					break;
			case 0x0A: c[10] += "Processor Card Slot";			break;
			case 0x0B: c[10] += "Proprietary Memory Card Slot";	break;
			case 0x0C: c[10] += "I/O Riser Card Slot";			break;
			case 0x0D: c[10] += "NuBus";						break;
			case 0x0E: c[10] += "PCI - 66MHz Capable";			break;
			case 0x0F: c[10] += "AGP";							break;
			case 0x10: c[10] += "AGP 2X";						break;
			case 0x11: c[10] += "AGP 4X";						break;
			case 0x12: c[10] += "PCI-X";						break;
			case 0x13: c[10] += "AGP 8X";						break;
			case 0xA0: c[10] += "PC-98/C20";					break;
			case 0xA1: c[10] += "PC-98/C24";					break;
			case 0xA2: c[10] += "PC-98/E";						break;
			case 0xA3: c[10] += "PC-98/Local Bus";				break;
			case 0xA4: c[10] += "PC-98/Card";					break;
			case 0xA5: c[10] += "PCI Express";					break;
			case 0xA6: c[10] += "PCI Express x1";				break;
			case 0xA7: c[10] += "PCI Express x2";				break;
			case 0xA8: c[10] += "PCI Express x4";				break;
			case 0xA9: c[10] += "PCI Express x8";				break;
			case 0xAA: c[10] += "PCI Express x16";				break;
			}
			c[10] += "\r\n";
			c[10] += "Slot Data Bus Width            : ";
			switch( p[0x6] ){
			case 0x01: c[10] += "Other";						break;
			case 0x02: c[10] += "Unknown";						break;
			case 0x03: c[10] += "8 bit";						break;
			case 0x04: c[10] += "16 bit";						break;
			case 0x05: c[10] += "32 bit";						break;
			case 0x06: c[10] += "64 bit";						break;
			case 0x07: c[10] += "128 bit";						break;
			case 0x08: c[10] += "1x or x1";						break;
			case 0x09: c[10] += "2x or x2";						break;
			case 0x0A: c[10] += "4x or x4";						break;
			case 0x0B: c[10] += "8x or x8";						break;
			case 0x0C: c[10] += "12x or x12";					break;
			case 0x0D: c[10] += "16x or x16";					break;
			case 0x0E: c[10] += "32x or x32";					break;
			}
			c[10] += "\r\n";
			c[10] += "Current Usage                  : ";
			switch( p[0x7] ){
			case 0x01: c[10] += "Other";						break;
			case 0x02: c[10] += "Unknown";						break;
			case 0x03: c[10] += "Available";					break;
			case 0x04: c[10] += "In use";						break;
			}
			c[10] += "\r\n";
			c[10] += "Slot Length                    : ";
			switch( p[0x8] ){
			case 0x01: c[10] += "Other";						break;
			case 0x02: c[10] += "Unknown";						break;
			case 0x03: c[10] += "Short Length";					break;
			case 0x04: c[10] += "Long Length";					break;
			}
			c[10] += "\r\n";
			c[10] += "Slot ID                        : " + DmiStringWX(MAKEWORD(p[0x9], p[0xA]), 2);
			c[10] += "Slot Characteristics 1         : " + DmiStringBX(p[0xB], 2);
			ch = p[0xB];
			if((ch >>  0) & 0x1){c[10] += " - ""Characteristics Unknown""\r\n";}
			if((ch >>  1) & 0x1){c[10] += " - ""Provides 5.0 Volts""\r\n";}
			if((ch >>  2) & 0x1){c[10] += " - ""Provides 3.3 Volts""\r\n";}
			if((ch >>  3) & 0x1){c[10] += " - ""Slot's opening is shared with another slot""\r\n";}
			if((ch >>  4) & 0x1){c[10] += " - ""PC Card slot supports PC Card-16""\r\n";}
			if((ch >>  5) & 0x1){c[10] += " - ""PC Card slot supports CardBus""\r\n";}
			if((ch >>  6) & 0x1){c[10] += " - ""PC Card slot supports Zoom Video""\r\n";}
			if((ch >>  7) & 0x1){c[10] += " - ""PC Card slot supports Modem Ring Resume""\r\n";}
			c[10] += "Slot Characteristics 2         : " + DmiStringBX(p[0xC], 2);
			ch = p[0xC];
			if((ch >>  0) & 0x1){c[10] += " - ""PCI slot supports Power Management Enable (PME#) signal""\r\n";}
			if((ch >>  1) & 0x1){c[10] += " - ""Slot supports hot-plug devices""\r\n";}
			if((ch >>  2) & 0x1){c[10] += " - ""PCI slot supports SMBus signal""\r\n";}
			if((ch >>  3) & 0x1){c[10] += " - ""Reserved""\r\n";}
			if((ch >>  4) & 0x1){c[10] += " - ""Reserved""\r\n";}
			if((ch >>  5) & 0x1){c[10] += " - ""Reserved""\r\n";}
			if((ch >>  6) & 0x1){c[10] += " - ""Reserved""\r\n";}
			if((ch >>  7) & 0x1){c[10] += " - ""Reserved""\r\n";}

			if(p[0x1] >= 0x10){
				c[10] += "Segment Group Number           : " + DmiStringWX(MAKEWORD(p[0xD], p[0xE]), 2);
				c[10] += "Bus Number                     : " + DmiStringB(p[0xF]);
				c[10] += "Device Number                  : " + DmiStringB((p[0x10] >> 3)); // bit 7:3
				c[10] += "Function Number                : " + DmiStringB((p[0x10] & 7)); // bit 3:0
			}
	
			if(flag[10]){m_Select.AddString(_T(" 09 System Slots"));flag[10] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 10 On Board Devices Information (obsolete)
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 10:
			c[11] += PrintHeader(dmi);
			if(flag[11]){m_Select.AddString(_T(" 10 On Board Devices Information  [don't support]"));flag[11] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 11 OEM Strings
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 11:
			c[12] += PrintHeader(dmi);
			c[12] += "Count                          : " + DmiStringBX(p[0x4]);
			for(i = 1; i <= p[0x4]; i++){
				cstr.Format(_T("%2d"),i);
				c[12] += "OEM String " + cstr + "                  : " + DmiString(dmi,i);
			}
			if(flag[12]){m_Select.AddString(_T(" 11 OEM Strings"));flag[12] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 12 System Configuration Options
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 12:
			c[13] += PrintHeader(dmi);
			c[13] += "Count                          : " + DmiStringBX(p[0x4]);
			if(flag[13]){m_Select.AddString(_T(" 12 System Configuration Options [don't support]"));flag[13] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 13 BIOS Language Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 13:
			c[14] += PrintHeader(dmi);
			c[14] += "Installable Languages          : " + DmiStringBX(p[0x4]);
			c[14] += "Flags                          : " + DmiStringBX(p[0x5], 2);
			ch = p[0x5];
			if((ch >>  0) & 0x1){
				c[14] += " - ""Abbreviated format""\r\n";
			}else{
				c[14] += " - ""Long format""\r\n";
			}
			/*
			c[14] += "Reserved                       : "+ DmiStringBX(p[0x6], 1, 0)
														+ DmiStringBX(p[0x7], 1, 0)
														+ DmiStringBX(p[0x8], 1, 0)
														+ DmiStringBX(p[0x9], 1, 0)
														+ DmiStringBX(p[0xA], 1, 0)
														+ DmiStringBX(p[0xB], 1, 0)
														+ DmiStringBX(p[0xC], 1, 0)
														+ DmiStringBX(p[0xD], 1, 0)
														+ DmiStringBX(p[0xE], 1, 0)
														+ DmiStringBX(p[0xF], 1, 0)
														+ DmiStringBX(p[0x10], 1, 0)
														+ DmiStringBX(p[0x11], 1, 0)
														+ DmiStringBX(p[0x12], 1, 0)
														+ DmiStringBX(p[0x13], 1, 0)
														+ DmiStringBX(p[0x14], 1);
			*/
			c[14] += "Current Languages              : " + DmiString(dmi, p[0x15]);
			for(i = 1; i <= p[0x4]; i++){
			c[14] += "Available Languages            : " + DmiString(dmi,i);
			}
			if(flag[14]){m_Select.AddString(_T(" 13 BIOS Language Information"));flag[14] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 14 Group Associations
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 14:
			c[15] += PrintHeader(dmi);
			if(flag[15]){m_Select.AddString(_T(" 14 Group Associations [don't support]"));flag[15] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 15 System Event Log
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 15:
			c[16] += PrintHeader(dmi);
			if(flag[16]){m_Select.AddString(_T(" 15 System Event Log [don't support]"));flag[16] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 16 Physical Memory Array
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 16:
			c[17] += PrintHeader(dmi);
			c[17] += "Location                       : ";
			switch (p[0x4]){
			case 0x01: c[17] += "Other";						break;
			case 0x02: c[17] += "Unknown";						break;
			case 0x03: c[17] += "System board or motherboard";	break;
			case 0x04: c[17] += "ISA add-on card";				break;
			case 0x05: c[17] += "EISA add-on card";				break;
			case 0x06: c[17] += "PCI add-on card";				break;
			case 0x07: c[17] += "MCA add-on card";				break;
			case 0x08: c[17] += "PCMCIA add-on card";			break;
			case 0x09: c[17] += "Proprietary add-on card";		break;
			case 0x0A: c[17] += "NuBus";						break;
			case 0xA0: c[17] += "PC-98/C20 add-on card";		break;
			case 0xA1: c[17] += "PC-98/C24 add-on card";		break;
			case 0xA2: c[17] += "PC-98/E add-on card";			break;
			case 0xA3: c[17] += "PC-98/Local bus add-on card";	break;
			}
			c[17] += "\r\n";
			c[17] += "Use                            : ";
			switch (p[0x5]){
			case 0x01: c[17] += "Other";						break;
			case 0x02: c[17] += "Unknown";						break;
			case 0x03: c[17] += "System memory";				break;
			case 0x04: c[17] += "Video memory";					break;
			case 0x05: c[17] += "Flash memory";					break;
			case 0x06: c[17] += "Non-volatile RAM";				break;
			case 0x07: c[17] += "Cache memory";					break;
			}
			c[17] += "\r\n";
			c[17] += "Memory Error Correction        : ";
			switch (p[0x6]){
			case 0x01: c[17] += "Other";						break;
			case 0x02: c[17] += "Unknown";						break;
			case 0x03: c[17] += "None";							break;
			case 0x04: c[17] += "Parity";						break;
			case 0x05: c[17] += "Single-bit ECC";				break;
			case 0x06: c[17] += "Multi-bit ECC";				break;
			case 0x07: c[17] += "CRC";							break;
			}
			c[17] += "\r\n";
			c[17] += "Maximum Capacity               : ";
			dword = MAKEDWORD(p[7], p[8], p[9], p[10]);
			if(dword == 0x80000000){
				c[17] += "Unknown";
			}else{
				cstr.Format(_T("%d MB"),dword / 1024);
				c[17] += cstr;
			}
			c[17] += "\r\n";
			c[17] += "Memory Error Information Handle: " + DmiStringWX(MAKEWORD(p[0xB], p[0xC]), 3); 
			c[17] += "Number of Memory Devices       : " + DmiStringWX(MAKEWORD(p[0xD], p[0xE])); 

			if(flag[17]){m_Select.AddString(_T(" 16 Physical Memory Array"));flag[17] = FALSE;}
			break;		
///////////////////////////////////////////////////////////////////////////////////////////////////
// 17 Memory Device
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 17:
			c[18] += PrintHeader(dmi);
			c[18] += "Physical Memory Array Handle   : " + DmiStringWX(MAKEWORD(p[0x4], p[0x5]), 3); 
			c[18] += "Memory Error Information Handle: " + DmiStringWX(MAKEWORD(p[0x6], p[0x7]), 3); 
			c[18] += "Total Width                    : ";
			word = MAKEWORD(p[0x8], p[0x9]);
			if(word == 0xFFFF){
				c[18] += "Unknown";
			}else{
				cstr.Format(_T("%d bit"), word);
				c[18] += cstr;
			}
			c[18] += "\r\n";
			c[18] += "Data Width                     : ";
			word = MAKEWORD(p[0xA], p[0xB]);
			if(word == 0xFFFF){
				c[18] += "Unknown";
			}else{
				cstr.Format(_T("%d bit"), word);
				c[18] += cstr;
			}
			c[18] += "\r\n";
			c[18] += "Size                           : ";
			word = MAKEWORD(p[0xC], p[0xD]);
			if(word == 0){
				c[18] += "No memory device";
			}else if(word == 0xFFFF){
				c[18] += "Unknown";
			}else if((word >> 15) & 0x1){
				cstr.Format(_T("%d KB"), (word & 0x7FFF));
				c[18] += cstr;
			}else{
				cstr.Format(_T("%d MB"), (word & 0x7FFF));
				c[18] += cstr;
			}
			c[18] += "\r\n";
			c[18] += "Form Factor                    : ";
			switch (p[0xE]){
			case 0x01: c[18] += "Other";			break;
			case 0x02: c[18] += "Unknown";			break;
			case 0x03: c[18] += "SIMM";				break;
			case 0x04: c[18] += "SIP";				break;
			case 0x05: c[18] += "Chip";				break;
			case 0x06: c[18] += "DIP";				break;
			case 0x07: c[18] += "ZIP";				break;
			case 0x08: c[18] += "Proprietary Card";	break;
			case 0x09: c[18] += "DIMM";				break;
			case 0x0A: c[18] += "TSOP";				break;
			case 0x0B: c[18] += "Row of chips";		break;
			case 0x0C: c[18] += "RIMM";				break;
			case 0x0D: c[18] += "SODIMM";			break;
			case 0x0E: c[18] += "SRIMM";			break;
			case 0x0F: c[18] += "FB-DIMM";			break;
			}
			c[18] += "\r\n";
			c[18] += "Device Set                     : " + DmiStringBX(p[0xF]);
			c[18] += "Device Locator                 : " + DmiString(dmi, p[0x10]);
			c[18] += "Bank Locator                   : " + DmiString(dmi, p[0x11]);

			c[18] += "Memory Type                    : ";
			switch (p[0x12]){
			case 0x01: c[18] += "Other";			break;
			case 0x02: c[18] += "Unknown";			break;
			case 0x03: c[18] += "DRAM";				break;
			case 0x04: c[18] += "EDRAM";			break;
			case 0x05: c[18] += "VRAM";				break;
			case 0x06: c[18] += "SRAM";				break;
			case 0x07: c[18] += "RAM";				break;
			case 0x08: c[18] += "ROM";				break;
			case 0x09: c[18] += "FLASH";			break;
			case 0x0A: c[18] += "EEPROM";			break;
			case 0x0B: c[18] += "FEPROM";			break;
			case 0x0C: c[18] += "EPROM";			break;
			case 0x0D: c[18] += "CDRAM";			break;
			case 0x0E: c[18] += "3DRAM";			break;
			case 0x0F: c[18] += "SDRAM";			break;
			case 0x10: c[18] += "SGRAM";			break;
			case 0x11: c[18] += "RDRAM";			break;
			case 0x12: c[18] += "DDR";				break;
			case 0x13: c[18] += "DDR2";				break;
			case 0x14: c[18] += "DDR2 FB-DIMM";		break;
			}
			c[18] += "\r\n";
			
			c[18] += "Type Detail                    : " + DmiStringWX(MAKEWORD(p[0x13], p[0x14]), 2);
			ch = MAKEWORD(p[0x13], p[0x14]);
			if((ch >>  0) & 0x1){c[18] += " - ""Reserved""\r\n";}
			if((ch >>  1) & 0x1){c[18] += " - ""Other""\r\n";}
			if((ch >>  2) & 0x1){c[18] += " - ""Unknown""\r\n";}
			if((ch >>  3) & 0x1){c[18] += " - ""Fast-paged""\r\n";}
			if((ch >>  4) & 0x1){c[18] += " - ""Static column""\r\n";}
			if((ch >>  5) & 0x1){c[18] += " - ""Pseudo-static""\r\n";}
			if((ch >>  6) & 0x1){c[18] += " - ""RAMBUS""\r\n";}
			if((ch >>  7) & 0x1){c[18] += " - ""Synchronous""\r\n";}
			if((ch >>  8) & 0x1){c[18] += " - ""CMOS""\r\n";}
			if((ch >>  9) & 0x1){c[18] += " - ""EDO""\r\n";}
			if((ch >> 10) & 0x1){c[18] += " - ""Window DRAM""\r\n";}
			if((ch >> 11) & 0x1){c[18] += " - ""Cache DRAM""\r\n";}
			if((ch >> 12) & 0x1){c[18] += " - ""Non-volatile""\r\n";}

			if(p[1] > 0x16){
				c[18] += "Speed                          : " + DmiStringWX(MAKEWORD(p[0x15], p[0x16]), 2);
			}
			if(p[1] > 0x17){
				c[18] += "Manufacturer                   : " + DmiString(dmi, p[0x17]);
			}
			if(p[1] > 0x18){
				c[18] += "Serial Number                  : " + DmiString(dmi, p[0x18]);
			}
			if(p[1] > 0x19){
				c[18] += "Asset Tag                      : " + DmiString(dmi, p[0x19]);
			}
			if(p[1] > 0x1A){
				c[18] += "Part Number                    : " + DmiString(dmi, p[0x1A]);
			}
			if(p[1] > 0x1B){
				c[18] += "Attributes                     : " + DmiStringB(p[0x1B]);
			}
			if(flag[18]){m_Select.AddString(_T(" 17 Memory Device"));flag[18] = FALSE;}
			break;	
///////////////////////////////////////////////////////////////////////////////////////////////////
// 18 32-bit Memory Error Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 18:
			c[19] += PrintHeader(dmi);
			if(flag[19]){m_Select.AddString(_T(" 18 32-bit Memory Error Information [don't support]"));flag[19] = FALSE;}
			break;	
///////////////////////////////////////////////////////////////////////////////////////////////////
// 19 Memory Array Mapped Address
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 19:
			c[20] += PrintHeader(dmi);
			c[20] += "Starting Address               : " + DmiStringDX(MAKEDWORD(p[0x4], p[0x5], p[0x6], p[0x7]), 2);
			c[20] += "Ending Address                 : " + DmiStringDX(MAKEDWORD(p[0x8], p[0x9], p[0xA], p[0xB]), 2);
			c[20] += "Memory Array Handle            : " + DmiStringWX(MAKEWORD(p[0xC], p[0xD]), 2);
			c[20] += "Partition Width                : " + DmiStringBX(p[0xE]);

			if(flag[20]){m_Select.AddString(_T(" 19 Memory Array Mapped Address"));flag[20] = FALSE;}
			break;				
///////////////////////////////////////////////////////////////////////////////////////////////////
// 20 Memory Device Mapped Address
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 20:
			c[21] += PrintHeader(dmi);
			c[21] += "Starting Address               : " + DmiStringDX(MAKEDWORD(p[0x4], p[0x5], p[0x6], p[0x7]), 2);
			c[21] += "Ending Address                 : " + DmiStringDX(MAKEDWORD(p[0x8], p[0x9], p[0xA], p[0xB]), 2);
			c[21] += "Memory Device Handle           : " + DmiStringWX(MAKEWORD(p[0xC], p[0xD]), 2);
			c[21] += "Mem Array Mapped Address Handle: " + DmiStringWX(MAKEWORD(p[0xE], p[0xF]), 2);
			c[21] += "Partition Row Position         : ";
			if(p[0x10] == 0xFF){
				c[21] += "Unknown\r\n";
			}else if(p[0x10] == 0x00){
				c[21] += "Reserved\r\n";
			}else{
				c[21] += DmiStringBX(p[0x10]);
			}
			c[21] += "Interleave Position            : ";
			if(p[0x11] == 0xFF){
				c[21] += "Unknown\r\n";
			}else if(p[0x11] == 0x00){
				c[21] += "Non-interleaved\r\n";
			}else{
				c[21] += DmiStringBX(p[0x11]);
			}
			c[21] += "Interleaved Data Depth         : ";
			if(p[0x12] == 0xFF){
				c[21] += "Unknown\r\n";
			}else if(p[0x12] == 0x00){
				c[21] += "Not part of an interleave\r\n";
			}else{
				c[21] += DmiStringBX(p[0x12]);
			}
			if(flag[21]){m_Select.AddString(_T(" 20 Memory Device Mapped Address"));flag[21] = FALSE;}
			break;	
///////////////////////////////////////////////////////////////////////////////////////////////////
// 21 Built-in Pointing Device
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 21:
			c[22] += PrintHeader(dmi);
			if(flag[22]){m_Select.AddString(_T(" 21 Built-in Pointing Device [don't support]"));flag[22] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 22 Portable Battery
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 22:
			c[23] += PrintHeader(dmi);
			if(flag[23]){m_Select.AddString(_T(" 22 Portable Battery [don't support]"));flag[23] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 23 System Reset
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 23:
			c[24] += PrintHeader(dmi);
			if(flag[24]){m_Select.AddString(_T(" 23 System Reset [don't support]"));flag[24] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 24 Hardware Security
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 24:
			c[25] += PrintHeader(dmi);
			if(flag[25]){m_Select.AddString(_T(" 24 Hardware Security [don't support]"));flag[25] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 25 System Power Controls
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 25:
			c[26] += PrintHeader(dmi);
			if(flag[26]){m_Select.AddString(_T(" 25 System Power Controls [don't support]"));flag[26] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 26 Voltage Probe
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 26:
			c[27] += PrintHeader(dmi);
			if(flag[27]){m_Select.AddString(_T(" 26 Voltage Probe [don't support]"));flag[27] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 27 Cooling Device
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 27:
			c[28] += PrintHeader(dmi);
			if(flag[28]){m_Select.AddString(_T(" 27 Cooling Device [don't support]"));flag[28] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 28 Temperature Probe
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 28:
			c[29] += PrintHeader(dmi);
			if(flag[29]){m_Select.AddString(_T(" 28 Temperature Probe [don't support]"));flag[29] = FALSE;}
			break;		
///////////////////////////////////////////////////////////////////////////////////////////////////
// 29 Electrical Current Probe
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 29:
			c[30] += PrintHeader(dmi);
			if(flag[30]){m_Select.AddString(_T(" 29 Electrical Current Probe [don't support]"));flag[30] = FALSE;}
			break;	
///////////////////////////////////////////////////////////////////////////////////////////////////
// 30 Out-of-Band Remote Access
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 30:
			c[31] += PrintHeader(dmi);
			if(flag[31]){m_Select.AddString(_T(" 30 Out-of-Band Remote Access [don't support]"));flag[31] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 31 Boot Integrity Services (BIS) Entry Point
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 31:
			c[32] += PrintHeader(dmi);
			if(flag[32]){m_Select.AddString(_T(" 31 Boot Integrity Services (BIS) Entry Point [don't support]"));flag[32] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 32 System Boot Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 32:
			c[33] += PrintHeader(dmi);
			/*
			c[33] += "Reserved                       : "+ DmiStringBX(p[0x4], 1, 0)
														+ DmiStringBX(p[0x5], 1, 0)
														+ DmiStringBX(p[0x6], 1, 0)
														+ DmiStringBX(p[0x7], 1, 0)
														+ DmiStringBX(p[0x8], 1, 0)
														+ DmiStringBX(p[0x9], 1);
			*/
			c[33] += "System Boot Status             : ";
			switch( p[0x0A] ){
			case 0x00: c[33] += "No errors detected";												break;
			case 0x01: c[33] += "No bootable media";												break;
			case 0x02: c[33] += "The \"normal\" operating system failed to load";					break;
			case 0x03: c[33] += "Firmware-detected hardware failure";								break;
			case 0x04: c[33] += "Operating system-detected hardware failure";						break;
			case 0x05: c[33] += "User-requested boot, usually via a keystroke";						break;
			case 0x06: c[33] += "System security violation";										break;
			case 0x07: c[33] += "Previously-requested image";										break;
			case 0x08: c[33] += "A system watchdog timer expired, causing the system to reboot";	break;
			}
			c[33] += "\r\n";
			if(flag[33]){m_Select.AddString(_T(" 32 System Boot Information"));flag[33] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 33 64-bit Memory Error Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 33:
			c[34] += PrintHeader(dmi);
			if(flag[34]){m_Select.AddString(_T(" 33 64-bit Memory Error Information [don't support]"));flag[34] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 34 Management Device
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 34:
			c[35] += PrintHeader(dmi);
			if(flag[35]){m_Select.AddString(_T(" 34 Management Device [don't support]"));flag[35] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 35 Management Device Component
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 35:
			c[36] += PrintHeader(dmi);
			if(flag[36]){m_Select.AddString(_T(" 35 Management Device Component [don't support]"));flag[36] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 36 Management Device Threshold Data
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 36:
			c[37] += PrintHeader(dmi);
			if(flag[37]){m_Select.AddString(_T(" 36 Management Device Threshold Data [don't support]"));flag[37] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 37 Memory Channel
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 37:
			c[38] += PrintHeader(dmi);
			if(flag[38]){m_Select.AddString(_T(" 37 Memory Channel [don't support]"));flag[38] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 38 IPMI Device Information
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 38:
			c[39] += PrintHeader(dmi);
			if(flag[39]){m_Select.AddString(_T(" 38 IPMI Device Information [don't support]"));flag[39] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 39 System Power Supply
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 39:
			c[40] += PrintHeader(dmi);
			if(flag[40]){m_Select.AddString(_T(" 39 System Power Supply [don't support]"));flag[40] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 40 Additional Information (2.6 or later)
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 40:
			c[41] += PrintHeader(dmi);
			if(flag[41]){m_Select.AddString(_T(" 40 Additional Information [don't support]"));flag[41] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 41 Additional Information (2.6 or later)
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 41:
			c[42] += PrintHeader(dmi);
			if(flag[42]){m_Select.AddString(_T(" 41 Onboard Devices Extended Information [don't support]"));flag[42] = FALSE;}
			break;
//////////////////////////////////////////////////////////////////////////////////////////////////
// 126 Inactive
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 126:
			c[127] += PrintHeader(dmi);
			if(flag[127]){m_Select.AddString(_T("126 Inactive"));flag[127] = FALSE;}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////
// 127 End-of-Table
///////////////////////////////////////////////////////////////////////////////////////////////////
		case 127:
			c[128] += PrintHeader(dmi);
			if(flag[128]){m_Select.AddString(_T("127 End-of-Table"));flag[128] = FALSE;}
			break;
}
		// next 
		next = p + dmi->Length;
		while(next[0] !=0 || next[1] != 0)
			next++;
		p = next + 2;
	}

	m_Select.AddString(_T(" All Information"));
	int number;

		c[ALL_INFORMATION_ID] += "\
----------------------------------------------------------------------\r\n\
%PRODUCT% %VERSION%%STATUS% (C) 2004-2010 hiyohiyo\r\n\
                          Crystal Dew World : http://crystalmark.info/\r\n\
----------------------------------------------------------------------\r\n\
";
	c[ALL_INFORMATION_ID].Replace(_T("%PRODUCT%"), CRYSTAL_DMI_PRODUCT);
	c[ALL_INFORMATION_ID].Replace(_T("%VERSION%"), CRYSTAL_DMI_VERSION);
	c[ALL_INFORMATION_ID].Replace(_T("%STATUS%"), CRYSTAL_DMI_STATUS);

	for(i=0; i < m_Select.GetCount() - 1; i++){
		m_Select.GetLBText(i, cstr);
		if(i != 0){
			number = _wtoi(cstr) + 1;
		}else{
			number = 0;
		}
		c[ALL_INFORMATION_ID] += 
"----------------------------------------------------------------------\r\n"
 + cstr + "\r\n" +
"----------------------------------------------------------------------\r\n"
 + c[number];
	}
	delete d;
	m_Select.SetCurSel(0);
	m_View.SetWindowText( c[ 0 ] );
	}catch(...){
		delete d;
		MessageBox(_T("Exception!! Please send me bug report."));
		EndDialog(0);
		return FALSE;
	}
	return TRUE;
}

void CCrystalDMIDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CCrystalDMIDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CCrystalDMIDlg::OnOK() 
{	
//	CDialog::OnOK();
}

void CCrystalDMIDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CCrystalDMIDlg::OnDestroy()
{
	CDialog::OnDestroy();
	DeinitOpenLibSys(&m_hOpenLibSys);
}

void CCrystalDMIDlg::OnSelchangeSelect() 
{
	CString cstr;
	int number;
	if(m_Select.GetCurSel() == m_Select.GetCount() - 1){
		number = ALL_INFORMATION_ID;
	}else if(m_Select.GetCurSel() != 0){
		m_Select.GetLBText(m_Select.GetCurSel(), cstr);
		number = _wtoi(cstr) + 1;
	}else{
		number = 0;
	}
	m_View.SetWindowText(c[ number ]);
}

void CCrystalDMIDlg::OnCopyText() 
{
	CString clip = c[ALL_INFORMATION_ID];
	if(OpenClipboard())
	{
		HGLOBAL clipbuffer;
		wchar_t * buffer;
		EmptyClipboard();
		int size = clip.GetLength() + 1;
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, size);
		buffer = (wchar_t*)GlobalLock(clipbuffer);
		_tcscpy_s(buffer, size, LPCTSTR(clip));
		GlobalUnlock(clipbuffer);
		SetClipboardData(CF_TEXT, clipbuffer);
		CloseClipboard();
	}	
}

/////////////////////////////////////////////////////////////////////////////
// Open/Execute File
/////////////////////////////////////////////////////////////////////////////
static void ExecuteFile(wchar_t* FileName)
{
	wchar_t path[MAX_PATH], file[MAX_PATH];
	wchar_t* ptrEnd;
	::GetModuleFileName(NULL, path, MAX_PATH);
	if((ptrEnd = wcsrchr(path, _T('\\'))) != NULL){
		*ptrEnd = '\0';
	}
	swprintf_s(file, MAX_PATH, _T("%s\\%s"), path, FileName);
	ShellExecute(NULL, NULL, file, NULL, NULL, SW_SHOWNORMAL);	
}

void CCrystalDMIDlg::OnCrystalDewWorld() 
{
	if(gLanguage == JAPANESE){
		ShellExecute(NULL, NULL, URL_JAPANESE, NULL, NULL, SW_SHOWNORMAL);
	}else{
		ShellExecute(NULL, NULL, URL_ENGLISH, NULL, NULL, SW_SHOWNORMAL);
	}	
}

void CCrystalDMIDlg::OnExit() 
{
	OnCancel();
}

void CCrystalDMIDlg::OnJapanese() 
{
	gLanguage = JAPANESE;
	CMenu *menu = GetMenu();

	menu->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, L"ファイル (&F)");
	menu->ModifyMenu(1, MF_BYPOSITION | MF_STRING, 1, L"編集 (&P)");
	menu->ModifyMenu(2, MF_BYPOSITION | MF_STRING, 2, L"ヘルプ (&H)");

	menu->ModifyMenu(IDM_EXIT, MF_STRING, IDM_EXIT, L"終了 (&E)\tAlt + F4");
	menu->ModifyMenu(IDM_COPY_TEXT, MF_STRING, IDM_COPY_TEXT, L"コピー (&C)\tCtrl + C");

	menu->CheckMenuItem(IDM_ENGLISH, MF_UNCHECKED);
	menu->CheckMenuItem(IDM_JAPANESE, MF_CHECKED);
	menu->EnableMenuItem(IDM_ENGLISH, MF_ENABLED);
	menu->EnableMenuItem(IDM_JAPANESE, MF_GRAYED);

	WritePrivateProfileString(_T("Setting"), _T("Language"), _T("jp"), m_ini);
	
	SetMenu(menu);	
}

void CCrystalDMIDlg::OnEnglish() 
{
	gLanguage = ENGLISH;
	CMenu *menu = GetMenu();

	menu->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, L"&File");
	menu->ModifyMenu(1, MF_BYPOSITION | MF_STRING, 1, L"&Edit");
	menu->ModifyMenu(2, MF_BYPOSITION | MF_STRING, 2, L"&Help");

	menu->ModifyMenu(IDM_EXIT, MF_STRING, IDM_EXIT, L"&Exit\tAlt + F4");
	menu->ModifyMenu(IDM_COPY_TEXT, MF_STRING, IDM_COPY_TEXT, L"&Copy\tCtrl + C");

	menu->CheckMenuItem(IDM_ENGLISH, MF_CHECKED);
	menu->CheckMenuItem(IDM_JAPANESE, MF_UNCHECKED);
	menu->EnableMenuItem(IDM_ENGLISH, MF_GRAYED);
	menu->EnableMenuItem(IDM_JAPANESE, MF_ENABLED);

	WritePrivateProfileString(_T("Setting"), _T("Language"), _T("en"), m_ini);
	SetMenu(menu);	
}


BOOL CCrystalDMIDlg::PreTranslateMessage(MSG* pMsg) 
{
	if(0 != ::TranslateAccelerator(m_hWnd, m_hAccelerator, pMsg))
	{
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL IsNT()
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	return (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
}
