// 080611_track_using_hough.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMy080611_track_using_houghApp:
// See 080611_track_using_hough.cpp for the implementation of this class
//

class CMy080611_track_using_houghApp : public CWinApp
{
public:
	CMy080611_track_using_houghApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMy080611_track_using_houghApp theApp;