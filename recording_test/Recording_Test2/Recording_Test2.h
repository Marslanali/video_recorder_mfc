
// Recording_Test2.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CRecording_Test2App:
// See Recording_Test2.cpp for the implementation of this class
//

class CRecording_Test2App : public CWinApp
{
public:
	CRecording_Test2App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CRecording_Test2App theApp;