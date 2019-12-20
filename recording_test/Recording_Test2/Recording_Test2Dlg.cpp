
// Recording_Test2Dlg.cpp : implementation file
//
#include <iostream>
#include "stdafx.h"
#include "Recording_Test2.h"
#include "Recording_Test2Dlg.h"
#include "afxdialogex.h"
#include <wmsysprf.h>
#include <wmsdk.h>
#include <mfapi.h>

#include <new>
#include <mfidl.h>            // Media Foundation interfaces
#include <mferror.h>        // Media Foundation error codes
#include <wmcontainer.h>    // ASF-specific components
#include <wmcodecdsp.h>        // Windows Media DSP interfaces
#include <Dmo.h>            // DMO objects
#include <uuids.h>            // Definition for FORMAT_VideoInfo
#include <propvarutil.h>
#include <wmsdkidl.h>
#include <string.h>
#include <Windows.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CComPtr<ICaptureGraphBuilder2> pBuilder;

BOOL CRecording_Test2Dlg::hrcheck(HRESULT hr, TCHAR* errtext)
{
	if (hr >= S_OK)
        return FALSE;
    TCHAR szErr[MAX_ERROR_TEXT_LEN];
    DWORD res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
	char Display_String [200];
    if (res)
	{
        _tprintf(_T("Error %x: %s\n%s\n"),hr, errtext,szErr);
		sprintf_s(Display_String, _T("\r\nError %x: %s: [%s]"), hr, errtext, szErr);
		arslanDisplay.Append(Display_String);
		UpdateData(FALSE);
	}
    else
	{
        _tprintf(_T("Error %x: %s\n"), hr, errtext);
		sprintf_s(Display_String, _T("\r\nError %x: %s "), hr, errtext);
		arslanDisplay.Append(Display_String);
		UpdateData(FALSE);
	}

	
    return TRUE;
}

CComPtr<IBaseFilter> CRecording_Test2Dlg::CreateFilterByName(const WCHAR* filterName, const GUID& category)
{
    HRESULT hr = S_OK;

    CComPtr<ICreateDevEnum> pSysDevEnum;
    hr = pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (hrcheck(hr, _T("Can't create System Device Enumerator")))
        return NULL;

    CComPtr<IEnumMoniker> pEnumCat;
    hr = pSysDevEnum->CreateClassEnumerator(category, &pEnumCat, 0);

    if (hr == S_OK) 
    {
        CComPtr<IMoniker> pMoniker;
        ULONG cFetched;
        while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
        {
            CComPtr<IPropertyBag> pPropBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if (SUCCEEDED(hr))
            {
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                if (SUCCEEDED(hr))
                {
                    if (wcscmp(filterName, varName.bstrVal)==0) {
                        CComPtr<IBaseFilter> pFilter;
                        hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
                        if (hrcheck(hr, _T("Can't bind moniker to filter object")))
                            return NULL;
                        return pFilter;
                    }
                }
                VariantClear(&varName);				
            }
            pMoniker.Release();
        }
    }
    return NULL;
}


CComPtr<IPin> CRecording_Test2Dlg::GetPin(IBaseFilter *pFilter, LPCOLESTR pinname)
{
    CComPtr<IEnumPins>  pEnum;
    CComPtr<IPin>       pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    
	if (hrcheck(hr, _T("Can't enumerate pins.")))
        return NULL;

    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_INFO pinfo;
        pPin->QueryPinInfo(&pinfo);
        BOOL found = !wcsicmp(pinname, pinfo.achName);
        if (pinfo.pFilter) pinfo.pFilter->Release();
        if (found)
            return pPin;
        pPin.Release();
    }
    printf("Pin not found!\n");
    return NULL;  
}
    


HRESULT CRecording_Test2Dlg::BuildGraph(IGraphBuilder *pGraph, LPCOLESTR dstFile1)
{
    
	HRESULT hr = S_OK;

    //graph builder
    CComPtr<ICaptureGraphBuilder2> pBuilder;
    hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
	CHECK_HR(hr, _T("Can't create Capture Graph Builder"));
    hr = pBuilder->SetFiltergraph(pGraph);
    CHECK_HR(hr, _T("Can't SetFiltergraph"));

    //add Smart Tee
    CComPtr<IBaseFilter> pSmartTee;
    hr = pSmartTee.CoCreateInstance(CLSID_SmartTee);
    CHECK_HR(hr, _T("Can't create Smart Tee"));
    hr = pGraph->AddFilter(pSmartTee, L"Smart Tee");
	CHECK_HR(hr, _T("Can't add Smart Tee to graph"));


    //add Video Renderer
    CComPtr<IBaseFilter> pVideoRenderer;
    hr = pVideoRenderer.CoCreateInstance(CLSID_VideoRenderer);
    CHECK_HR(hr, _T("Can't create Video Renderer"));
    hr = pGraph->AddFilter(pVideoRenderer, L"Video Renderer");
    CHECK_HR(hr, _T("Can't add Video Renderer to graph"));

    //add AVI Decompressor
    CComPtr<IBaseFilter> pAVIDecompressor;
    hr = pAVIDecompressor.CoCreateInstance(CLSID_AVIDec);
    CHECK_HR(hr, _T("Can't create AVI Decompressor"));
    hr = pGraph->AddFilter(pAVIDecompressor, L"AVI Decompressor");
    CHECK_HR(hr, _T("Can't add AVI Decompressor to graph"));


    //add Microphone (USB Audio Device)
    CComPtr<IBaseFilter> pMicrophoneUSBAudioDevice = CreateFilterByName(L"Microphone (USB Audio Device)", CLSID_AudioCaptureSources);
    hr = pGraph->AddFilter(pMicrophoneUSBAudioDevice, L"Microphone (USB Audio Device)");
    CHECK_HR(hr, _T("Can't add Microphone (USB Audio Device) to graph"));
	
    //add Smart Tee
    CComPtr<IBaseFilter> pSmartTee2;
    hr = pSmartTee2.CoCreateInstance(CLSID_SmartTee);
    CHECK_HR(hr, _T("Can't create Smart Tee"));
    hr = pGraph->AddFilter(pSmartTee2, L"Smart Tee");
    CHECK_HR(hr, _T("Can't add Smart Tee to graph"));

    //connect Microphone (USB Audio Device) and Smart Tee
    hr = pGraph->ConnectDirect(GetPin(pMicrophoneUSBAudioDevice, L"Capture"), GetPin(pSmartTee2, L"Input"), NULL);
    CHECK_HR(hr, _T("Can't connect Microphone (USB Audio Device) and Smart Tee"));

	//add Default DirectSound Device
    CComPtr<IBaseFilter> pDefaultDirectSoundDevice;
    hr = pDefaultDirectSoundDevice.CoCreateInstance(CLSID_DSoundRender);
    CHECK_HR(hr, _T("Can't create Default DirectSound Device"));
    hr = pGraph->AddFilter(pDefaultDirectSoundDevice, L"Default DirectSound Device");
    CHECK_HR(hr, _T("Can't add Default DirectSound Device to graph"));


	/*
	//add Speakers / Headphones (Realtek 
    CComPtr<IBaseFilter> pSpeakersHeadphonesRealtek;
    hr = pSpeakersHeadphonesRealtek.CoCreateInstance(CLSID_AudioRender);
    CHECK_HR(hr, _T("Can't create Speakers / Headphones (Realtek "));
    hr = pGraph->AddFilter(pSpeakersHeadphonesRealtek, L"Speakers / Headphones (Realtek ");
    CHECK_HR(hr, _T("Can't add Speakers / Headphones (Realtek  to graph"));
	*/

	/*
    //add Speakers (2- High Definition Au
    CComPtr<IBaseFilter> pSpeakers2HighDefinitionAu;
    hr = pSpeakers2HighDefinitionAu.CoCreateInstance(CLSID_AudioRender);
    CHECK_HR(hr, _T("Can't create Speakers (2- High Definition Au"));
    hr = pGraph->AddFilter(pSpeakers2HighDefinitionAu, L"Speakers (2- High Definition Au");
    CHECK_HR(hr, _T("Can't add Speakers (2- High Definition Au to graph"));

	*/

    //add Compressor
    CComPtr<IBaseFilter> pCompressor;
    hr = pCompressor.CoCreateInstance(CLSID_DMOWrapperFilter);
    CHECK_HR(hr, _T("Can't create DMO Wrapper"));
    CComQIPtr<IDMOWrapperFilter, &IID_IDMOWrapperFilter> pCompressor_wrapper(pCompressor);
    if (!pCompressor_wrapper)
        CHECK_HR(E_NOINTERFACE, _T("Can't get IDMOWrapperFilter"));
    hr = pCompressor_wrapper->Init(CLSID_Compressor, CLSID_Compressor_cat);
    CHECK_HR(hr, _T("DMO Wrapper Init failed"));
    hr = pGraph->AddFilter(pCompressor, L"Compressor");
    CHECK_HR(hr, _T("Can't add Compressor to graph"));


    //connect Smart Tee and Compressor
    hr = pGraph->ConnectDirect(GetPin(pSmartTee2, L"Capture"), GetPin(pCompressor, L"in0"), NULL);
    CHECK_HR(hr, _T("Can't connect Smart Tee and Compressor"));


    //connect Smart Tee and Speakers (2- High Definition Au
    hr = pGraph->ConnectDirect(GetPin(pSmartTee2, L"Preview"), GetPin(pDefaultDirectSoundDevice, L"Audio Input pin (rendered)"), NULL);
    CHECK_HR(hr, _T("Can't connect Smart Tee and Speakers (2- High Definition Au"));


    //add USB Video Device
    CComPtr<IBaseFilter> pUSBVideoDevice2 = CreateFilterByName(L"USB Video Device", CLSID_VideoCaptureSources);
    hr = pGraph->AddFilter(pUSBVideoDevice2, L"USB Video Device");
    CHECK_HR(hr, _T("Can't add USB Video Device to graph"));

    
	//connect USB Video Device and Smart Tee
    hr = pGraph->ConnectDirect(GetPin(pUSBVideoDevice2, L"Capture"), GetPin(pSmartTee, L"Input"), NULL);
    CHECK_HR(hr, _T("Can't connect USB Video Device and Smart Tee"));


    //connect Smart Tee and AVI Decompressor
    hr = pGraph->ConnectDirect(GetPin(pSmartTee, L"Preview"), GetPin(pAVIDecompressor, L"XForm In"), NULL);
    CHECK_HR(hr, _T("Can't connect Smart Tee and AVI Decompressor"));


    //connect AVI Decompressor and Video Renderer
    hr = pGraph->ConnectDirect(GetPin(pAVIDecompressor, L"XForm Out"), GetPin(pVideoRenderer, L"Input"), NULL);
    CHECK_HR(hr, _T("Can't connect AVI Decompressor and Video Renderer"));


    //add WM ASF Writer
    CComPtr<IBaseFilter> pWMASFWriter2;
    hr = pWMASFWriter2.CoCreateInstance(CLSID_WMAsfWriter);
    CHECK_HR(hr, _T("Can't create WM ASF Writer"));
    hr = pGraph->AddFilter(pWMASFWriter2, L"WM ASF Writer");
    CHECK_HR(hr, _T("Can't add WM ASF Writer to graph"));
	
	//---------------------------------------------------------------

	WM_MEDIA_TYPE* pMediaType = NULL;
	DWORD cbMediaType = 0;
	
	IWMStreamConfig* pIWMStreamConfig = NULL;
    
	CComPtr<IConfigAsfWriter> pConfigAsfWriter2;
	hr = pWMASFWriter2->QueryInterface( IID_IConfigAsfWriter, (void**)&pConfigAsfWriter2 );
	CHECK_HR(hr, _T("Can't query Configure WM ASF Writer")); 
	
	CComPtr<IWMProfileManager> pProfileMgr;
	hr = WMCreateProfileManager(&pProfileMgr);
	CHECK_HR(hr, _T("Can't create Profile Manager")); 
	
	CComPtr<IWMProfileManager2> pProfileMgr2;
	hr = pProfileMgr->QueryInterface( IID_IWMProfileManager2,(void**)&pProfileMgr2);
	CHECK_HR(hr, _T("Can't create Profile Manager 2")); 

	hr = pProfileMgr2->SetSystemProfileVersion( WMT_VER_8_0 );
	CHECK_HR(hr, _T("Can't Set System Profile Version")); 
	
	CComPtr<IWMProfile> pWMProfile;
	hr = pProfileMgr->LoadProfileByID(WMProfile_V80_700PALVideo,&pWMProfile);
	CHECK_HR(hr, _T("Can't Load Profile By ID")); 
	
	hr = pWMProfile->GetStream(1,&pIWMStreamConfig);
	CHECK_HR(hr, _T("Can't stream WM ASF Profile"));

	CComQIPtr<IWMMediaProps, &IID_IWMMediaProps> pIWMMediaProps = pIWMStreamConfig;
	hr = pIWMMediaProps->GetMediaType(pMediaType, &cbMediaType);
	
	pMediaType = (WM_MEDIA_TYPE*)new char [cbMediaType];
											
	hr = pIWMMediaProps->GetMediaType(pMediaType, &cbMediaType);

	BITMAPINFOHEADER* pbmih = NULL;
	if (WMFORMAT_VideoInfo == pMediaType->formattype)
	{
		WMVIDEOINFOHEADER* pvih = (WMVIDEOINFOHEADER*)pMediaType->pbFormat;
		pbmih = &pvih->bmiHeader;
	}
	else if (WMFORMAT_MPEG2Video == pMediaType->formattype)
	{
														
		WMVIDEOINFOHEADER2* pvih = (WMVIDEOINFOHEADER2*)&((WMMPEG2VIDEOINFO*)pMediaType->pbFormat)->hdr;
		pbmih = &pvih->bmiHeader;
	}

	// modify the video dimensions, set the property, reconfigure the stream
	// and then configure the ASF writer with this modified profile
	pbmih->biWidth = 640;	// was 320;
	pbmih->biHeight = 480;	// was 240;
	pbmih->biSizeImage = pbmih->biWidth * pbmih->biHeight * pbmih->biBitCount / 8;	// NOTE: This calculation is not correct for all bit depths
	hr = pIWMMediaProps->SetMediaType(pMediaType);

	hr = pWMProfile->ReconfigStream(pIWMStreamConfig);
	CHECK_HR(hr, _T("Can't ReConfigure WM ASF Profile")); 

	hr = pConfigAsfWriter2->ConfigureFilterUsingProfile(pWMProfile);
	CHECK_HR(hr, _T("Can't Configure WM ASF Profile")); 

	//---------------------------------------------------------------

	CComPtr<IAMCameraControl> pCameraControl;
	//CComPtr<IBaseFilter> pUSBVideoDevice2 = CreateFilterByName(L"USB Video Device", CLSID_VideoCaptureSources);

	hr = pUSBVideoDevice2->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
	CHECK_HR(hr, _T("Can't query Configure camera control")); 
	
	long Min, Max, Step, Default, Flags, Val;

	CComPtr<IAMVideoProcAmp> pProcAmp;
	// Query the capture filter for the IAMVideoProcAmp interface.
	//IAMVideoProcAmp *pProcAmp = 0;
	hr = pUSBVideoDevice2->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
	CHECK_HR(hr, _T("Can't query INTERFAC of CAMERA control")); 
		
	// Get the range and default values 
	hr = pProcAmp->GetRange(VideoProcAmp_Brightness, &Min, &Max, &Step, &Default, &Flags);
	CHECK_HR(hr, _T("Can't query VideoProcAmp_Brightness of CAMERA control")); 
	
	//hr = pProcAmp->GetRange(VideoProcAmp_BacklightCompensation, &Min, &Max, &Step, &Default, &Flags);
	hr = pProcAmp->GetRange(VideoProcAmp_Contrast, &Min, &Max, &Step, &Default, &Flags);
	CHECK_HR(hr, _T("Can't query VideoProcAmp_Contrast of CAMERA control")); 
	
	hr = pProcAmp->GetRange(VideoProcAmp_Saturation, &Min, &Max, &Step, &Default, &Flags);
	CHECK_HR(hr, _T("Can't query VideoProcAmp_Saturation of CAMERA control")); 
	
	//hr = pProcAmp->GetRange(VideoProcAmp_Sharpness, &Min, &Max, &Step, &Default, &Flags);
	//hr = pProcAmp->GetRange(VideoProcAmp_WhiteBalance, &Min, &Max, &Step, &Default, &Flags);

	hr = pProcAmp->Set(VideoProcAmp_Brightness,20 , VideoProcAmp_Flags_Manual);
	CHECK_HR(hr, _T("Can't set VideoProcAmp_Brightness of CAMERA control")); 	
	
	//hr = pProcAmp->Set(VideoProcAmp_BacklightCompensation, 0, VideoProcAmp_Flags_Manual);
	hr = pProcAmp->Set(VideoProcAmp_Gamma,0,VideoProcAmp_Flags_Manual);
	CHECK_HR(hr, _T("Can't set VideoProcAmp_Gamma of CAMERA control")); 

	hr = pProcAmp->Set(VideoProcAmp_Hue, 46, VideoProcAmp_Flags_Manual);
	CHECK_HR(hr, _T("Can't set VideoProcAmp_Hue of CAMERA control")); 
	
	hr = pProcAmp->Set(VideoProcAmp_Contrast, 109, VideoProcAmp_Flags_Manual);
	CHECK_HR(hr, _T("Can't set VideoProcAmp_Contrast of CAMERA control")); 
	
	hr = pProcAmp->Set(VideoProcAmp_Saturation, 100, VideoProcAmp_Flags_Manual);
	CHECK_HR(hr, _T("Can't set VideoProcAmp_Saturation of CAMERA control")); 
	
	//hr = pProcAmp->Set(VideoProcAmp_Sharpness, 0, VideoProcAmp_Flags_Manual);
	//hr = pProcAmp->Set(VideoProcAmp_WhiteBalance, 2800, VideoProcAmp_Flags_Manual);

	//---------------------------------------------------------------

	//set destination filename
    CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> pWMASFWriter2_sink(pWMASFWriter2);
    if (!pWMASFWriter2_sink)
        CHECK_HR(E_NOINTERFACE, _T("Can't get IFileSinkFilter"));
    hr = pWMASFWriter2_sink->SetFileName(dstFile1, NULL);
    CHECK_HR(hr, _T("Can't set filename"));

    //connect Compressor and WM ASF Writer
    hr = pGraph->ConnectDirect(GetPin(pCompressor, L"out0"), GetPin(pWMASFWriter2, L"Audio Input 01"), NULL);
    CHECK_HR(hr, _T("Can't connect Compressor and WM ASF Writer"));


    //connect Smart Tee and WM ASF Writer
    hr = pGraph->ConnectDirect(GetPin(pSmartTee, L"Capture"), GetPin(pWMASFWriter2, L"Video Input 01"), NULL);
    CHECK_HR(hr, _T("Can't connect Smart Tee and WM ASF Writer"));
		
	return S_OK;
}


CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRecording_Test2Dlg dialog




CRecording_Test2Dlg::CRecording_Test2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRecording_Test2Dlg::IDD, pParent)
	, arslanDisplay(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRecording_Test2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, arslanDisplay);
}

BEGIN_MESSAGE_MAP(CRecording_Test2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CRecording_Test2Dlg::OnBnClickedButton1)
//	ON_BN_CLICKED(IDC_BUTTON2, &CRecording_Test2Dlg::OnBnClickedButton2)
ON_EN_CHANGE(IDC_EDIT1, &CRecording_Test2Dlg::OnEnChangeEdit1)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CRecording_Test2Dlg::OnNMCustomdrawSlider1)
ON_BN_CLICKED(IDC_BUTTON80, &CRecording_Test2Dlg::OnBnClickedButton80)
END_MESSAGE_MAP()


// CRecording_Test2Dlg message handlers

BOOL CRecording_Test2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRecording_Test2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRecording_Test2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRecording_Test2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRecording_Test2Dlg::OnBnClickedButton1()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	
	CoInitialize(NULL);
    CComPtr<IGraphBuilder> graph;
    graph.CoCreateInstance(CLSID_FilterGraph);

    std::cout<<"Building Graph \n....."<<std::endl;
    HRESULT hr = BuildGraph(graph, L"C:\\Users\\arslan\\Desktop\\arslanali1992_with_box-6.mp4");

    if (hr==S_OK) {
		std::cout<<"\n Running Graph .....\n"<<std::endl;
		CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl(graph);
        hr = mediaControl->Run();
		
        CComQIPtr<IMediaEvent, &IID_IMediaEvent> mediaEvent(graph);
        BOOL stop = FALSE;
        MSG msg;
        while(!stop) 
        {
            long ev=0, p1=0, p2=0;
            Sleep(500);
          //  printf(".");
            while(PeekMessage(&msg, NULL, 0,0, PM_REMOVE))
                DispatchMessage(&msg);
            while (mediaEvent->GetEvent(&ev, (LONG_PTR*)&p1, (LONG_PTR*)&p2, 0)==S_OK)
            {
                if (ev == EC_COMPLETE || ev == EC_USERABORT)
                {
           std::cout<<"Done .....\n"<<std::endl;
                    stop = TRUE;
                }
                else
                if (ev == EC_ERRORABORT)
                {
					std::cout<<"An Error has occured ...\n"<<std::endl;
                    mediaControl->Stop();
                    stop = TRUE;
                }
                mediaEvent->FreeEventParams(ev, p1, p2);
            }
        }
    }
    CoUninitialize();
    
    

}

/*
void CRecording_Test2Dlg::OnBnClickedButton2()
{
	

	
	// TODO: Add your control notification handler code here
}
*/
void CRecording_Test2Dlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}



void CRecording_Test2Dlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void CRecording_Test2Dlg::OnBnClickedButton80()
{
	// TODO: Add your control notification handler code here
	#pragma omp parallel
    {
#pragma omp task
        //OnBnClickedButton1();
#pragma omp task
       // OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
    }
}
