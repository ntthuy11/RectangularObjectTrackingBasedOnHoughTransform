#pragma once

#include "cv.h"
#include "highgui.h"

#define HALF_PI (1.5707963) 

class TetragonDetection
{
public:
	TetragonDetection(void);
	~TetragonDetection(void);

	void detectRectangle(CvSeq* linesWithRhoTheta, double thrForTheta, CvSeq* linesForRect);
	CvPoint getCenterPointOfLinesForRect();
	int getLeftMostXOfLinesForRect();
	int getRightMostXOfLinesForRect();
	int getTopMostYOfLinesForRect();
	int getBottomMostYOfLinesForRect();

private:
	CvPoint centerPointOfLinesForRect;
	int leftMostX, rightMostX, topMostY, bottomMostY;

	void makeIndexForParallelPairs(CvSeq* linesWithRhoTheta, double thrForTheta, CvSeq* parallelPairIndices);
	void eliminateDisturbedLines(CvSeq* linesWithRhoTheta, double thrForTheta, CvSeq* parallelPairIndices);
	void findLinesForRectangle(CvSeq* linesWithRhoTheta, double thrForTheta, CvSeq* parallelPairIndices, CvSeq* linesForRect);
	void eliminateDisturbedLinesForRect(CvSeq* linesForRect);

	double calculateX(double rho1, double rho2, double theta1, double theta2);
	double calculateY(double rho1, double theta1, double x);
	int findNearestValueWithMultiplication(int n, int coeff);
};
