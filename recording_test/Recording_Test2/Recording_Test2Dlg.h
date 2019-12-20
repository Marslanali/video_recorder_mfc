
// Recording_Test2Dlg.h : header file
//


//#pragma once
#include <DShow.h>
#include <atlbase.h>
#include <initguid.h>
#include <dvdmedia.h>
#include <dmodshow.h> // we're going to use DMO Wrapper Filter
#include <dmoreg.h>
#include <dshowasf.h>


//change this macro to fit your style of error handling
#define CHECK_HR(hr, msg) if (hrcheck(hr, msg)) return hr;
//change this macro to fit your style of error handling


//#pragma comment(lib,"dmoguids.lib")

// The required link libraries 
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "msdmo")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "propsys")






// {33D9A762-90C8-11D0-BD43-00A0C911CE86}
DEFINE_GUID(CLSID_AudioCaptureSources,
0x33D9A762, 0x90C8, 0x11D0, 0xBD, 0x43, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86); //

/*
// {B87BEB7B-8D29-423F-AE4D-6582C10175AC}
DEFINE_GUID(CLSID_VideoRenderer,
0xB87BEB7B, 0x8D29, 0x423F, 0xAE, 0x4D, 0x65, 0x82, 0xC1, 0x01, 0x75, 0xAC); //quartz.dll
*/

// {EF011F79-4000-406D-87AF-BFFB3FC39D57}
DEFINE_GUID(CLSID_Compressor,
0xEF011F79, 0x4000, 0x406D, 0x87, 0xAF, 0xBF, 0xFB, 0x3F, 0xC3, 0x9D, 0x57); //DMO


// {F3602B3F-0592-48DF-A4CD-674721E7EBEB}
DEFINE_GUID(CLSID_Compressor_cat,
0xF3602B3F, 0x0592, 0x48DF, 0xA4, 0xCD, 0x67, 0x47, 0x21, 0xE7, 0xEB, 0xEB); //DMO category


// {860BB310-5D01-11D0-BD3B-00A0C911CE86}
DEFINE_GUID(CLSID_VideoCaptureSources,
0x860BB310, 0x5D01, 0x11D0, 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86); //



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};



// CRecording_Test2Dlg dialog
class CRecording_Test2Dlg : public CDialogEx
{
// Construction
public:
	CRecording_Test2Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RECORDING_TEST2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	BOOL hrcheck(HRESULT hr, TCHAR* errtext);
	CComPtr<IBaseFilter> CreateFilterByName(const WCHAR* filterName, const GUID& category);
    CComPtr<IPin> GetPin(IBaseFilter *pFilter, LPCOLESTR pinname);
	HRESULT BuildGraph(IGraphBuilder *pGraph, LPCOLESTR dstFile1);

	afx_msg void OnBnClickedButton2();
	afx_msg void OnEnChangeEdit1();
	CString arslanDisplay;


	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton80();
};
