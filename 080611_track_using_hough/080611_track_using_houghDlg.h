// 080611_track_using_houghDlg.h : header file
//

#pragma once
#include "cv.h"
#include "highgui.h"

// CMy080611_track_using_houghDlg dialog
class CMy080611_track_using_houghDlg : public CDialog
{
// Construction
public:
	CMy080611_track_using_houghDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MY080611_TRACK_USING_HOUGH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	void trackCircle();
	void detectRectangle(IplImage* grayImgRegion, IplImage* edgeImgRegion, IplImage* resultImgRegion, 
		bool isDrawToSrcImg = false, IplImage* srcImg = 0, CvPoint winTopLeft = cvPoint(-1, -1));
	void cropImg(IplImage* src, CvRect roi, IplImage* des);

public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonQuit();
	afx_msg void OnBnClickedButtonInit();
	afx_msg void OnBnClickedButtonLoadImg();
	afx_msg void OnBnClickedButtonLoadVideo();
	afx_msg void OnBnClickedButtonGenNoiseBackgrd();
	afx_msg void OnBnClickedButtonVideoToImg();
	afx_msg void OnBnClickedButtonSpecialInvert();
};
