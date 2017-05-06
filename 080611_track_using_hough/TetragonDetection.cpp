#include "StdAfx.h"
#include "TetragonDetection.h"

#define FRAME_WIDTH		(320)
#define FRAME_HEIGHT	(240)

TetragonDetection::TetragonDetection(void) { }
TetragonDetection::~TetragonDetection(void) { }

void TetragonDetection::detectRectangle(CvSeq* linesWithRhoTheta, double thrForTheta, CvSeq* linesForRect) {	

	// ------- tim kiem cac duong thang song song voi nhau, ghep lai thanh 1 cap -------
	CvMemStorage* ppiStorage = cvCreateMemStorage(0);
	CvSeq* parallelPairIndices = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(CvPoint), ppiStorage);
	makeIndexForParallelPairs(linesWithRhoTheta, thrForTheta, parallelPairIndices);

	// test
	/*for(int i = 0; i < parallelPairIndices->total; i++) {	
		CvPoint* lr = (CvPoint*)cvGetSeqElem(parallelPairIndices, i);
		CvPoint startPoint = { lr->x, lr->y };
	}*/


	// ---------------------------------------------------------------------------------- 
	if (parallelPairIndices->total > 0) {
		eliminateDisturbedLines(linesWithRhoTheta, thrForTheta, parallelPairIndices);
		findLinesForRectangle(linesWithRhoTheta, thrForTheta, parallelPairIndices, linesForRect);
		eliminateDisturbedLinesForRect(linesForRect);
	}

	cvReleaseMemStorage(&ppiStorage);
}

CvPoint TetragonDetection::getCenterPointOfLinesForRect()	{	return centerPointOfLinesForRect;	}
int TetragonDetection::getLeftMostXOfLinesForRect()			{	return leftMostX;					}
int TetragonDetection::getRightMostXOfLinesForRect()		{	return rightMostX;					}
int TetragonDetection::getTopMostYOfLinesForRect()			{	return topMostY;					}
int TetragonDetection::getBottomMostYOfLinesForRect()		{	return bottomMostY;					}

// ======================================== PRIVATE ========================================

void TetragonDetection::makeIndexForParallelPairs(CvSeq* linesWithRhoTheta, double thrForTheta, CvSeq* parallelPairIndices) {
	int size = linesWithRhoTheta->total;

	for(int i = 0; i < size; i++) {
		CvPoint2D64f* lineI = (CvPoint2D64f*)cvGetSeqElem(linesWithRhoTheta, i);
		double thetaI = lineI->y;

		for(int j = i + 1; j < size; j++) {
			CvPoint2D64f* lineJ = (CvPoint2D64f*)cvGetSeqElem(linesWithRhoTheta, j);
			double thetaJ = lineJ->y;

			if(fabs(thetaI - thetaJ) <= thrForTheta) {
				CvPoint pairIndex = { i, j }; 
				cvSeqPush(parallelPairIndices, &pairIndex);
			}
		}
	}
}

void TetragonDetection::eliminateDisturbedLines(CvSeq* linesWithRhoTheta, double thrForTheta, CvSeq* parallelPairIndices) {
	
	// loai nhung line gan nhau (degree gan nhau) & khong vuong goc voi nhau. Lay line dau tien lam chuan (gia su nhu line dau tien, line 0, la chinh xac 100%)
	CvPoint* pairIndex = (CvPoint*)cvGetSeqElem(parallelPairIndices, 0);
	double theta = ((CvPoint2D64f*)cvGetSeqElem(linesWithRhoTheta, pairIndex->x))->y;

	for(int i = 1; i < parallelPairIndices->total; i++) {
		CvPoint* pairIndexN = (CvPoint*)cvGetSeqElem(parallelPairIndices, i);
		double thetaN = ((CvPoint2D64f*)cvGetSeqElem(linesWithRhoTheta, pairIndexN->x))->y;
		if (fabs(theta - thetaN) <= thrForTheta) cvSeqRemove(parallelPairIndices, i);
	}

	// tim ra line chuan vuong goc voi line chuan 0. Tu line chuan vuong goc nay, loai nhung line gan nhau (degree gan nhau)
	double thetaPerpendicular = -1;
	bool foundThePerpendicular = false;
	for(int i = 1; i < parallelPairIndices->total; i++) {
		CvPoint* pairIndexN = (CvPoint*)cvGetSeqElem(parallelPairIndices, i);
		double thetaN = ((CvPoint2D64f*)cvGetSeqElem(linesWithRhoTheta, pairIndexN->x))->y;
	
		if (!foundThePerpendicular) {
			double dev = fabs(theta - thetaN);
			bool isPerpendicular = (HALF_PI - thrForTheta <= dev && dev <= HALF_PI + thrForTheta);
			if (isPerpendicular) {
				thetaPerpendicular = thetaN; 
				foundThePerpendicular = true;
			}
		} else {
			if (fabs(thetaPerpendicular - thetaN) <= thrForTheta) cvSeqRemove(parallelPairIndices, i);
		}
	}
}

void TetragonDetection::findLinesForRectangle(CvSeq* linesWithRhoTheta, double thrForTheta, CvSeq* parallelPairIndices, CvSeq* linesForRect) {
	int size = parallelPairIndices->total;

	for(int i = 0; i < size; i++) {
		CvPoint* pairIndex = (CvPoint*)cvGetSeqElem(parallelPairIndices, i);

		for(int c = 0; c <= 1; c++) {
			int index = pairIndex->x;
			if(c == 1) index = pairIndex->y;

			CvPoint2D64f* rhoAndTheta = (CvPoint2D64f*)cvGetSeqElem(linesWithRhoTheta, index);
			double rho = rhoAndTheta->x, theta = rhoAndTheta->y;

			for(int j = 0; j < size; j++) {
				if (i != j) {
					CvPoint* pairIndexN = (CvPoint*)cvGetSeqElem(parallelPairIndices, j);
					int x1, x2, y1, y2;
					x1 = x2 = y1 = y2 = -1;

					// ---------- pairIndex->n with pairIndexN->x ----------
					CvPoint2D64f* rhoAndThetaN = (CvPoint2D64f*)cvGetSeqElem(linesWithRhoTheta, pairIndexN->x);
					double rhoN = rhoAndThetaN->x, thetaN = rhoAndThetaN->y;

					double dev = fabs(theta - thetaN);
					bool isPerpendicular = (HALF_PI - thrForTheta <= dev && dev <= HALF_PI + thrForTheta);
					if (isPerpendicular) {
						x1 = (int)calculateX(rho, rhoN, theta, thetaN);						
						if (i < j) y1 = (int)calculateY(rho, theta, x1);
						else y1 = (int)calculateY(rhoN, thetaN, x1);
					}

					// ---------- pairIndex->n with pairIndexN->y ----------
					rhoAndThetaN = (CvPoint2D64f*)cvGetSeqElem(linesWithRhoTheta, pairIndexN->y);
					rhoN = rhoAndThetaN->x;	thetaN = rhoAndThetaN->y;

					dev = fabs(theta - thetaN);
					isPerpendicular = (HALF_PI - thrForTheta <= dev && dev <= HALF_PI + thrForTheta);
					if (isPerpendicular) {
						x2 = (int)calculateX(rho, rhoN, theta, thetaN);
						if (i < j) y2 = (int)calculateY(rho, theta, x2);
						else y2 = (int)calculateY(rhoN, thetaN, x2);
					}

					// ---------- create line for 2 intersect points ----------
					if (x1 != -1 && x2 != -1) {
						CvRect line = { x1, y1, x2, y2 }; 
						cvSeqPush(linesForRect, &line);
					}
				}
			}
		}
	}
}

void TetragonDetection::eliminateDisturbedLinesForRect(CvSeq* linesForRect) {
	int sumX = 0, sumY = 0;
	int nPoints = 0;

	leftMostX = 1000;
	rightMostX = -1;
	topMostY = 1000;
	bottomMostY = -1;

	for(int i = 0; i < linesForRect->total; i++) {
		CvRect* line = (CvRect*)cvGetSeqElem(linesForRect, i);			
		bool point1x_biggerThan0 = (line->x >= 0);		bool point1x_smallerThanFrameWidth = (line->x < FRAME_WIDTH);
		bool point1y_biggerThan0 = (line->y >= 0);		bool point1y_smallerThanFrameHeight = (line->y < FRAME_HEIGHT);
		bool point2x_biggerThan0 = (line->width >= 0);	bool point2x_smallerThanFrameWidth = (line->width < FRAME_WIDTH);
		bool point2y_biggerThan0 = (line->height >= 0);	bool point2y_smallerThanFrameHeight = (line->height < FRAME_HEIGHT);
		if (point1x_biggerThan0 && leftMostX > line->x)						leftMostX = line->x;				// co loai tru outlier (nhung toa do < 0 va > chieu dai/rong cua anh)
		if (point2x_biggerThan0 && leftMostX > line->width)					leftMostX = line->width;
		if (point1x_smallerThanFrameWidth && rightMostX < line->x)			rightMostX = line->x;
		if (point2x_smallerThanFrameWidth && rightMostX < line->width)		rightMostX = line->width;
		if (point1y_biggerThan0 && topMostY > line->y)						topMostY = line->y;
		if (point2y_biggerThan0 && topMostY > line->height)					topMostY = line->height;			
		if (point1y_smallerThanFrameHeight && bottomMostY < line->y)  		bottomMostY = line->y;
		if (point2y_smallerThanFrameHeight && bottomMostY < line->height)	bottomMostY = line->height;

		if ( (point1x_biggerThan0 && point1x_smallerThanFrameWidth) && (point1y_biggerThan0 && point1y_smallerThanFrameHeight) 
		  && (point2x_biggerThan0 && point2x_smallerThanFrameWidth) && (point2y_biggerThan0 && point2y_smallerThanFrameHeight)) {				
			sumX += line->x + line->width;
			sumY += line->y + line->height;
			nPoints += 2;
		} else {
			//cvSeqRemove(linesForRect, i);

			// remove di nhung line co lien quan.
			// vi cu 4 line lien tiep nhau (tinh tu 0) la dai dien cho 1 rectangle, 
			// nen neu co 1 line trong khoang [n, n+3] khong hop le thi xoa luon nhung line con lai trong khoang nay
			int nearestVal = findNearestValueWithMultiplication(i, 4);
			for (int j = nearestVal; j < nearestVal + 4; j++)
				cvSeqRemove(linesForRect, nearestVal);
		}
	}

	if (nPoints > 0) // tinh trong tam cua hinh CN (cung la tam cua cua so quet)
		centerPointOfLinesForRect = cvPoint(sumX / nPoints, sumY / nPoints); // gia su la chac chan tim duoc hinh CN, nen linesForRect->total luon >= 4
	else 
		centerPointOfLinesForRect = cvPoint(-1, -1);
}

double TetragonDetection::calculateX(double rho1, double rho2, double theta1, double theta2) {
	double sinTheta1 = sin(theta1);
	double sinTheta2 = sin(theta2);
	return (rho2*sinTheta1 - rho1*sinTheta2) / (cos(theta2)*sinTheta1 - cos(theta1)*sinTheta2);
}

double TetragonDetection::calculateY(double rho1, double theta1, double x) {
	return (rho1 - x*cos(theta1)) / sin(theta1);
}

int TetragonDetection::findNearestValueWithMultiplication(int n, int coeff) {
	int result = -1;
	
	for (int i = 1; i <= n; i++)
		if (i*coeff > n) {
			result = (i - 1) * coeff;
			break;
		}

	if (result < 0) result = 0;

	return result;
}