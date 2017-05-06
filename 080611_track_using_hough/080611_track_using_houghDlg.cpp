// 080611_track_using_houghDlg.cpp : implementation file
//

#include "stdafx.h"
#include "080611_track_using_hough.h"
#include "080611_track_using_houghDlg.h"
#include "StandardHough.h"
#include "TetragonDetection.h"
#include "UniformGen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// --------------------------------
#define WIN_SRC_IMG		"Source Image"
#define WIN_RESULT_IMG	"Result Image"

#define FRAME_WIDTH		(320)
#define FRAME_HEIGHT	(240)

//#define FRAME_WIDTH		(200)
//#define FRAME_HEIGHT	(160)

#define SQRT_2_DIV_2	(0.707107)	// ca(n 2 chia 2 = cos45


StandardHough stdHough;
TetragonDetection tetraDetect;
CUniformGen uniformGen;

IplImage *currFrame, *grayImg, *edgeImg, *resultImg;

CvCapture* capture;
bool running;

CvMemStorage *linesForRectStorage, *previousLinesForRectStorage;
CvSeq *linesForRect, *previousLinesForRect;

// --------------------------------

CMy080611_track_using_houghDlg::CMy080611_track_using_houghDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMy080611_track_using_houghDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMy080611_track_using_houghDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMy080611_track_using_houghDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_START, &CMy080611_track_using_houghDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMy080611_track_using_houghDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_QUIT, &CMy080611_track_using_houghDlg::OnBnClickedButtonQuit)
	ON_BN_CLICKED(IDC_BUTTON_INIT, &CMy080611_track_using_houghDlg::OnBnClickedButtonInit)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_IMG, &CMy080611_track_using_houghDlg::OnBnClickedButtonLoadImg)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_VIDEO, &CMy080611_track_using_houghDlg::OnBnClickedButtonLoadVideo)
	ON_BN_CLICKED(IDC_BUTTON_GEN_NOISE_BACKGRD, &CMy080611_track_using_houghDlg::OnBnClickedButtonGenNoiseBackgrd)
	ON_BN_CLICKED(IDC_BUTTON_VIDEO_TO_IMG, &CMy080611_track_using_houghDlg::OnBnClickedButtonVideoToImg)
	ON_BN_CLICKED(IDC_BUTTON_SPECIAL_INVERT, &CMy080611_track_using_houghDlg::OnBnClickedButtonSpecialInvert)
END_MESSAGE_MAP()


// CMy080611_track_using_houghDlg message handlers

BOOL CMy080611_track_using_houghDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMy080611_track_using_houghDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMy080611_track_using_houghDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// =====================================================================================

void CMy080611_track_using_houghDlg::OnBnClickedButtonInit() {
	//capture = cvCaptureFromCAM(0);
	cvNamedWindow(WIN_SRC_IMG, CV_WINDOW_AUTOSIZE);
	cvNamedWindow(WIN_RESULT_IMG, CV_WINDOW_AUTOSIZE);

	grayImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	edgeImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	resultImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 3);

	linesForRectStorage = cvCreateMemStorage(0);			linesForRect = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(CvRect), linesForRectStorage);
	previousLinesForRectStorage = cvCreateMemStorage(0);	previousLinesForRect = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(CvRect), previousLinesForRectStorage);


	// ----- khoi tao cho giao dien ----
	CComboBox *combo = (CComboBox *) this->GetDlgItem(IDC_COMBO_SPECIFIED_VIDEO);
	combo->AddString(L"00- move horizontally (no noise)");
	combo->AddString(L"01- move vertically (no noise)");
	combo->AddString(L"02- move vertically (noise_150_1000)");
	combo->AddString(L"03- move diagonally (no noise)");
	combo->AddString(L"04- move diagonally (noise_150_3000)");
	combo->AddString(L"05- move diagonally (noise_150_5000)");
	combo->AddString(L"06- move diagonally (noise_200_8000)");
	combo->AddString(L"07- move diagonally (noise_200_10000)**");
	combo->AddString(L"08- move diagonally (noise_200_8000_rect)");
	combo->AddString(L"09- move diagonally (noise_200_8000_rect_diag)**");
	combo->AddString(L"10- move diagonally (noise_200_8000_rect_diag2)");
	combo->AddString(L"11- real 1 (61 frames)");
	combo->AddString(L"12- real 2 (81 frames)");
	combo->SetCurSel(0);
}

void CMy080611_track_using_houghDlg::OnBnClickedButtonStart() {	
	running = true;
	while(running) {
		currFrame = cvQueryFrame(capture);	
		//trackCircle();

		cvCvtColor(currFrame, grayImg, CV_BGR2GRAY);
		cvCanny(grayImg, edgeImg, 50, 200);//		cvFlip(edgeImg, edgeImg);		cvShowImage(WIN_SRC_IMG, edgeImg);
		cvSetImageROI(edgeImg, cvRect(60, 40, 200, 160));
		detectRectangle(grayImg, edgeImg, resultImg);

		cvFlip(resultImg, resultImg);
		//cvRectangle(resultImg, cvPoint(160 - 100, 120 - 80), cvPoint(160 + 100, 120 + 80), CV_RGB(0, 0, 255));
		cvShowImage(WIN_RESULT_IMG, resultImg);
		cvWaitKey(5); 
	}
}

void CMy080611_track_using_houghDlg::OnBnClickedButtonStop() {
	running = false;
}

void CMy080611_track_using_houghDlg::OnBnClickedButtonQuit() { 
	running = false;

	cvDestroyAllWindows();
	//cvReleaseImage(&currFrame); // khong release duoc
	cvReleaseImage(&grayImg);
	cvReleaseImage(&edgeImg);
	cvReleaseImage(&resultImg);

	cvReleaseCapture(&capture);
	cvReleaseMemStorage(&linesForRectStorage);
	cvReleaseMemStorage(&previousLinesForRectStorage);

	OnOK();
}

void CMy080611_track_using_houghDlg::OnBnClickedButtonGenNoiseBackgrd() {
	CGaussianGen genNoisePosX, genNoisePosY;
	double noisePosSigma = 200;
	int noiseMax = 8000;

	IplImage* genImg = cvCreateImage(cvSize(FRAME_WIDTH, FRAME_HEIGHT), IPL_DEPTH_8U, 1);
	cvZero(genImg);

	srand((unsigned)time(NULL));	
	double seed = rand()*1.0/RAND_MAX;		genNoisePosX.setSeed(seed, 0, noisePosSigma);
	seed = rand()*1.0/RAND_MAX;				genNoisePosY.setSeed(seed, 0, noisePosSigma);

	for(int i = 0; i < noiseMax; i++) {
		int noisePosX = int(genNoisePosX.rnd() + 0.5);		if (noisePosX < 0 || FRAME_WIDTH <= noisePosX) noisePosX = (int) ( (double)rand() / ((double)(RAND_MAX) + (double)(1)) * FRAME_WIDTH );;
		int noisePosY = int(genNoisePosY.rnd() + 0.5);		if (noisePosY < 0 || FRAME_HEIGHT <= noisePosY) noisePosY = (int) ( (double)rand() / ((double)(RAND_MAX) + (double)(1)) * FRAME_HEIGHT );;
		genImg->imageData[noisePosY*genImg->widthStep + noisePosX] = 255;
	}
	cvSaveImage("backgrd.bmp", genImg);

	cvReleaseImage(&genImg);
}

void CMy080611_track_using_houghDlg::OnBnClickedButtonVideoToImg() {
	char filenameInMultiByte[256];
	CFileDialog dlg(TRUE, _T("*.avi"), _T(""), OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,_T("Image (*.avi)|*.avi|All Files (*.*)|*.*||"),NULL);	
	if (dlg.DoModal() != IDOK) return;		
	WideCharToMultiByte(CP_ACP, 0, dlg.GetPathName(), -1, filenameInMultiByte, 256, NULL, NULL);
	
	int i = 0;
	capture = cvCaptureFromAVI(filenameInMultiByte);

	while(cvGrabFrame(capture)) { 		
		currFrame = cvRetrieveFrame(capture); 

		// extract to src images
		CString s; s.Format(L"%03d.bmp", i++); CStringA fn (s); cvSaveImage(fn, currFrame);

		// extract to edge images
		/*cvCvtColor(currFrame, grayImg, CV_BGR2GRAY);
		cvFlip(grayImg, grayImg);
		cvCanny(grayImg, edgeImg, 50, 200);
		CString s; s.Format(L"%03d.bmp", i++); CStringA fn (s); cvSaveImage(fn, edgeImg);*/
	}
	MessageBox(L"Video -> Images. Finish!");
}

void CMy080611_track_using_houghDlg::OnBnClickedButtonSpecialInvert() {
	char filenameInMultiByte[256];
	CFileDialog dlg(TRUE, _T("*.avi"), _T(""), OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,_T("Image (*.bmp)|*.bmp|All Files (*.*)|*.*||"),NULL);	
	if (dlg.DoModal() != IDOK) return;		
	WideCharToMultiByte(CP_ACP, 0, dlg.GetPathName(), -1, filenameInMultiByte, 256, NULL, NULL);

	IplImage* img = cvvLoadImage(filenameInMultiByte);
	uchar* imgData = (uchar *)img->imageData;
	int height = img->height, width = img->width, step = img->widthStep, channels = img->nChannels;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
				int position = i*step + j*channels;
				int position1 = i*step + j*channels + 1;
				int position2 = i*step + j*channels + 2;
				if (imgData[position] == 0 && imgData[position1] == 0 && imgData[position2] == 0) // mau den
					imgData[position] = imgData[position1] = imgData[position2] = 255;
				else if (imgData[position] == 255 && imgData[position1] == 255 && imgData[position2] == 255) // mau trang
					imgData[position] = imgData[position1] = imgData[position2] = 0;
			}
	}
	
	// save image
	CString s; s.Format(L"%03d.bmp", 0); CStringA fn (s); cvSaveImage(fn, img);

	cvReleaseImage(&img);
}

void CMy080611_track_using_houghDlg::OnBnClickedButtonLoadImg() {
	char filenameInMultiByte[256];
	CFileDialog dlg(TRUE, _T("*.avi"), _T(""), OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,_T("Image (*.bmp)|*.bmp|All Files (*.*)|*.*||"),NULL);	
	if (dlg.DoModal() != IDOK) return;		
	WideCharToMultiByte(CP_ACP, 0, dlg.GetPathName(), -1, filenameInMultiByte, 256, NULL, NULL);	
	
	cvCvtColor(cvvLoadImage(filenameInMultiByte), grayImg, CV_BGR2GRAY);
	cvCanny(grayImg, edgeImg, 50, 200);
	cvShowImage(WIN_SRC_IMG, edgeImg);

	detectRectangle(grayImg, edgeImg, resultImg);
	cvShowImage(WIN_RESULT_IMG, resultImg);

	cvClearSeq(linesForRect);
}

void CMy080611_track_using_houghDlg::OnBnClickedButtonLoadVideo() {

	// ================================== chinh toa do & load video ==================================
	int rectWidth = -1;		int rectHeight = -1;
	int winBorder = -1;
	int winCenterX = -1;	int winCenterY = -1;
	CString s;

	switch (((CComboBox *) GetDlgItem(IDC_COMBO_SPECIFIED_VIDEO))->GetCurSel()) {
		case 0: // move horizontally (no noise)
			rectWidth = 74;		rectHeight = 53;	winBorder = 16;		winCenterX = 190;	winCenterY = 107;			
			s = L"..\\testdata\\synthetic\\1-di chuyen ngang\\1-nonoise.avi";
			break;
		case 1:	// move vertically (no noise)
			rectWidth = 74;		rectHeight = 53;	winBorder = 16;		winCenterX = 160;	winCenterY = 95;
			s = L"..\\testdata\\synthetic\\2-di chuyen dung\\1-nonoise.avi";
			break;
		case 2:	// move vertically (noise_150_1000)
			rectWidth = 74;		rectHeight = 53;	winBorder = 16;		winCenterX = 165;	winCenterY = 85;
			s = L"..\\testdata\\synthetic\\2-di chuyen dung\\2-noise_150_1000.avi";
			break;
		case 3:	// move diagonally (no noise)
			rectWidth = 93;		rectHeight = 67;	winBorder = 26;		winCenterX = 72;	winCenterY = 170;
			s = L"..\\testdata\\synthetic\\3-di chuyen xeo\\1-nonoise.avi";
			break;
		case 4:	// move diagonally (noise_150_3000)
			rectWidth = 93;		rectHeight = 67;	winBorder = 18;		winCenterX = 72;	winCenterY = 170;
			s = L"..\\testdata\\synthetic\\3-di chuyen xeo\\2-noise_150_3000.avi";
			break;
		case 5:	// move diagonally (noise_150_5000)
			rectWidth = 93;		rectHeight = 67;	winBorder = 18;		winCenterX = 72;	winCenterY = 170;
			s = L"..\\testdata\\synthetic\\3-di chuyen xeo\\3-noise_150_5000.avi";
			break;
		case 6:	// move diagonally (noise_200_8000)
			rectWidth = 93;		rectHeight = 67;	winBorder = 14;		winCenterX = 68;	winCenterY = 166;
			s = L"..\\testdata\\synthetic\\3-di chuyen xeo\\4-noise_200_8000.avi";
			break;
		case 7:	// move diagonally (noise_200_10000)
			rectWidth = 93;		rectHeight = 67;	winBorder = 14;		winCenterX = 52;	winCenterY = 170;
			s = L"..\\testdata\\synthetic\\3-di chuyen xeo\\5-noise_200_10000.avi";
			break;
		case 8:	// move diagonally (noise_200_8000_rect)
			rectWidth = 93;		rectHeight = 67;	winBorder = 14;		winCenterX = 62;	winCenterY = 166;
			s = L"..\\testdata\\synthetic\\3-di chuyen xeo\\6-noise_200_8000_rect.avi";
			break;
		case 9:	// move diagonally (noise_200_8000_rect_diag)
			rectWidth = 86;		rectHeight = 79;	winBorder = 18;		winCenterX = 64;	winCenterY = 155;
			s = L"..\\testdata\\synthetic\\3-di chuyen xeo\\7-noise_200_8000_rect_diag.avi";
			break;
		case 10:	// move diagonally (noise_200_8000_rect_diag2)
			rectWidth = 72;		rectHeight = 58;	winBorder = 14;		winCenterX = 80;	winCenterY = 170;
			s = L"..\\testdata\\synthetic\\3-di chuyen xeo\\8-noise_200_8000_rect_diag2.avi";
			break;
		case 11:	// real 1 (61 frames)
			rectWidth = 117;	rectHeight = 140;	winBorder = 15;		winCenterX = 181;	winCenterY = 115;
			s = L"..\\testdata\\real\\1\\edge_61frames.avi";
			break;
		case 12:	// real 2 (81 frames)
			rectWidth = 114;	rectHeight = 108;	winBorder = 20;		winCenterX = 134;	winCenterY = 121;
			s = L"..\\testdata\\real\\2\\edge_81frames.avi";
			break;
	}

	int twoWinBorder = 2*winBorder;
	int winWidth = rectWidth + twoWinBorder; 			int winHeight = rectHeight + twoWinBorder;

	int halfOfWinWidth = winWidth/2;					int halfOfWinHeight = winHeight/2;
	int winLeftTopX = winCenterX - halfOfWinWidth;		int winLeftTopY = winCenterY - halfOfWinHeight;
	int winRightBottomX = winCenterX + halfOfWinWidth;	int winRightBottomY = winCenterY + halfOfWinHeight;


	// ================================== chay ==================================
	CStringA filename (s);
	capture = cvCaptureFromAVI(filename);

	int i = 0; // dung de save image
	while(cvGrabFrame(capture)) { 
		currFrame = cvRetrieveFrame(capture);		 cvFlip(cvCloneImage(currFrame), resultImg);
		cvCvtColor(currFrame, grayImg, CV_BGR2GRAY);
		cvFlip(grayImg, grayImg);


		// ----- khoi tao bien -----
		int imgRegionWidth = winRightBottomX - winLeftTopX + 1;	
		int imgRegionHeight = winRightBottomY - winLeftTopY + 1;
		IplImage* grayImgRegion = cvCreateImage(cvSize(imgRegionWidth, imgRegionHeight), IPL_DEPTH_8U, 1);
		IplImage* resultImgRegion = cvCreateImage(cvSize(imgRegionWidth, imgRegionHeight), IPL_DEPTH_8U, 3);
		cropImg(grayImg, cvRect(winLeftTopX, winLeftTopY, winRightBottomX, winRightBottomY), grayImgRegion);


		// ----- detect rectangle o frame nay & ve ket qua (cac lines) len do -----
		bool isDrawToSrcImg = true;
		detectRectangle(grayImgRegion, grayImgRegion, resultImgRegion, isDrawToSrcImg, resultImg, cvPoint(winLeftTopX, winLeftTopY));

		if (linesForRect->total == 0) { // neu ko detect duoc lines nao thi cu ve tam len do cac lines o frame cu (dung de test & tao nen su tron tru khi chay)
			for(int i = 0; i < previousLinesForRect->total; i++) {	
				CvRect* line = (CvRect*)cvGetSeqElem(previousLinesForRect, i);
				CvPoint startPoint = { line->x, line->y };
				CvPoint endPoint = { line->width, line->height };
				cvLine(resultImgRegion, startPoint, endPoint, CV_RGB(255, 255, 0), 1);
				if (isDrawToSrcImg) 
					cvLine(resultImg, cvPoint(winLeftTopX + startPoint.x, winLeftTopY + startPoint.y), cvPoint(winLeftTopX + endPoint.x, winLeftTopY + endPoint.y), CV_RGB(255, 255, 0), 1);
			}
		} else {
			previousLinesForRect = cvCloneSeq(linesForRect);
		}
		cvShowImage(WIN_SRC_IMG, resultImg);
		cvShowImage(WIN_RESULT_IMG, resultImgRegion);
		//CString s; s.Format(L"%03d.bmp", i++); CStringA fn (s); cvSaveImage(fn, resultImg);


		// ----- tinh trong tam cua hinh CN (cung la tam cua cua so quet) & kich thuoc cua so -----
		CvPoint centerOfRect = tetraDetect.getCenterPointOfLinesForRect();
		if (centerOfRect.x != -1) {
			winCenterX = (winCenterX - imgRegionWidth/2) + centerOfRect.x;
			winCenterY = (winCenterY - imgRegionHeight/2) + centerOfRect.y;

			int currFrameWidth_1 = FRAME_WIDTH - 1;
			int currFrameHeight_1 = FRAME_HEIGHT - 1;
			halfOfWinWidth = (tetraDetect.getRightMostXOfLinesForRect() - tetraDetect.getLeftMostXOfLinesForRect())/2 + winBorder;
			halfOfWinHeight = (tetraDetect.getBottomMostYOfLinesForRect() - tetraDetect.getTopMostYOfLinesForRect())/2 + winBorder;
			winLeftTopX = winCenterX - halfOfWinWidth;			winLeftTopX = (winLeftTopX < 0 ? 0 : winLeftTopX);
			winLeftTopY = winCenterY - halfOfWinHeight;			winLeftTopY = (winLeftTopY < 0 ? 0 : winLeftTopY);
			winRightBottomX = winCenterX + halfOfWinWidth;		winRightBottomX = (winRightBottomX > currFrameWidth_1 ? currFrameWidth_1 : winRightBottomX);
			winRightBottomY = winCenterY + halfOfWinHeight;		winRightBottomY = (winRightBottomY > currFrameHeight_1 ? currFrameHeight_1 : winRightBottomY);
		}


		// ----- release -----
		cvReleaseImage(&grayImgRegion);
		cvReleaseImage(&resultImgRegion);
		cvClearSeq(linesForRect);

		cvWaitKey(100);
	}
}

// ================================= PRIVATE =================================

/*void CMy080611_track_using_houghDlg::trackLine() { // TESTING ONLY
	cvCvtColor(currFrame, grayImg, CV_BGR2GRAY);	// gray img
	cvCanny(grayImg, edgeImg, 50, 200);				// edge img
	resultImg = cvCloneImage(currFrame);			// result img

	CvMemStorage* storage = cvCreateMemStorage(0);
	double rho = 1.0, theta = 0.01;
	int threshold = 100, linesMax = 20;

	CvSeq* lines = cvHoughLines2(edgeImg, storage, CV_HOUGH_STANDARD, rho, theta, threshold);	
	//lines = cvHoughLines2(edgeImg, storage, CV_HOUGH_MULTI_SCALE, rho, theta, threshold, 1, 0.01); // khong biet xai the nao
	for(int i = 0; i < MIN(lines->total, linesMax); i++) {
		float* line = (float*)cvGetSeqElem(lines,i);
		float rho = line[0], theta = line[1];
		CvPoint pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000*(-b));	pt2.x = cvRound(x0 - 1000*(-b));
		pt1.y = cvRound(y0 + 1000*(a));     pt2.y = cvRound(y0 - 1000*(a));
		cvLine(resultImg, pt1, pt2, CV_RGB(255,0,0), 1, 8);
	} 

	cvReleaseMemStorage(&storage);
}*/

void CMy080611_track_using_houghDlg::trackCircle() {
	cvCvtColor(currFrame, grayImg, CV_BGR2GRAY);	// gray img
	cvSmooth(grayImg, grayImg, CV_GAUSSIAN, 5, 5);  // smooth it, otherwise a lot of false circles may be detected // http://opencvlibrary.sourceforge.net/CvReference
	resultImg = cvCloneImage(currFrame);			// result img // khong co lay edge image nhu hay lam (vi HoughCircles co lam roi!?)

	CvMemStorage* storage = cvCreateMemStorage(0);

	double resolution = 2; // resolution of the accumulator used to detect centers of the circles (2 is a good value)
	double minDistance = grayImg->height/4.0; // minimum distance between centers of the detected circles
	CvSeq* circles = cvHoughCircles(grayImg, storage, CV_HOUGH_GRADIENT, resolution, minDistance, 200, 100);

	for(int i = 0; i < circles->total; i++) {
         float* p = (float*)cvGetSeqElem(circles, i);
         cvCircle(resultImg, cvPoint(cvRound(p[0]),cvRound(p[1])), 3, CV_RGB(0,255,0), -1, 8, 0);
         cvCircle(resultImg, cvPoint(cvRound(p[0]),cvRound(p[1])), cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0);
    }

	cvReleaseMemStorage(&storage);
	cvReleaseImage(&grayImg);	
}

void CMy080611_track_using_houghDlg::detectRectangle(IplImage* grayImgRegion, IplImage* edgeImgRegion, IplImage* resultImgRegion, 
													 bool isDrawToSrcImg, IplImage* srcImg, CvPoint winTopLeft) {

	// ------------------ line detect ------------------
	CvMemStorage* linesWithRhoThetaStorage = cvCreateMemStorage(0);
	CvSeq* linesWithRhoTheta = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(CvPoint2D64f), linesWithRhoThetaStorage); // sizeof(CvRect) dung cho muc dich ve ra

	double diagonal = sqrt( pow(edgeImgRegion->width*1.0, 2) + pow(edgeImgRegion->height*1.0, 2) );
	double radiusSmall = diagonal/10;
	int threshold = int(diagonal/4);
	//double rho = 0.75;			// chon tu dong
	//double theta = 2.35619 / (srcImg->width > srcImg->height ? srcImg->height : srcImg->width);
	int linesMax = 6;
	stdHough.run(edgeImgRegion, radiusSmall, linesWithRhoTheta, 1, 0.01, threshold, linesMax);

	// dung de ve ra => test
	cvCvtColor(grayImgRegion, resultImgRegion, CV_GRAY2BGR);
	for (int i = 0; i < linesWithRhoTheta->total; i++) {
		CvPoint2D64f* rhoAndTheta = (CvPoint2D64f*)cvGetSeqElem(linesWithRhoTheta, i);
		double rho = rhoAndTheta->x, theta = rhoAndTheta->y;
		double cosTheta = cos(theta), sinTheta = sin(theta);
		double x0 = cosTheta*rho, y0 = sinTheta*rho;
		CvPoint startPoint = { cvRound(x0 + 1000*(-sinTheta)), cvRound(y0 + 1000*(cosTheta)) };
		CvPoint endPoint = { cvRound(x0 - 1000*(-sinTheta)), cvRound(y0 - 1000*(cosTheta)) };
		cvLine(resultImgRegion, startPoint, endPoint, CV_RGB(0, 255, 0), 1);
	}

	
	// ------------------ rectangle detect ------------------
	double thrForTheta = 0.1;
	tetraDetect.detectRectangle(linesWithRhoTheta, thrForTheta, linesForRect);
	
	// dung de ve ra => test
	//cvCvtColor(grayImgRegion, resultImgRegion, CV_GRAY2BGR);
	for(int i = 0; i < linesForRect->total; i++) {	
		CvRect* line = (CvRect*)cvGetSeqElem(linesForRect, i);
		CvPoint startPoint = { line->x, line->y };
		CvPoint endPoint = { line->width, line->height };
		cvLine(resultImgRegion, startPoint, endPoint, CV_RGB(255, 0, 0), 2);

		if (isDrawToSrcImg) {
			cvLine(srcImg, 
				cvPoint(winTopLeft.x + startPoint.x, winTopLeft.y + startPoint.y), 
				cvPoint(winTopLeft.x + endPoint.x, winTopLeft.y + endPoint.y), 				
				CV_RGB(255, 0, 0), 2);
		}
	}


	// ------------------ release ------------------
	cvReleaseMemStorage(&linesWithRhoThetaStorage);	
}

void CMy080611_track_using_houghDlg::cropImg(IplImage* src, CvRect roi, IplImage* des) {
	int srcStep = src->widthStep;	const uchar* srcData = (uchar *)src->imageData;
	int desStep = des->widthStep;	uchar* desData = (uchar *)des->imageData;
	for(int i = roi.y; i <= roi.height; i++) for(int j = roi.x; j <= roi.width; j++) for(int k = 0; k < src->nChannels; k++) 
		desData[(i-roi.y)*desStep + (j-roi.x)*src->nChannels + k] = srcData[i*srcStep + j*src->nChannels + k];
}


