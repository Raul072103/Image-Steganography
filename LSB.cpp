// utils.cpp
#include "LSB.h"
#include "stdafx.h"
#include "common.h"
#include "utils.h"

#include <iostream>
#include <opencv2/core/utils/logger.hpp>


// This function should be called only if the safety checks have been made and the secret is small enough to be encoded.
Mat encode_grayscale(Mat src, byte* secret, int secretLength, int noBits) {

	int height = src.rows;
	int width = src.cols;
	Mat dst = Mat(height, width, CV_8UC1);

	int currentBit = 0;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			
			uchar grayLevel = src.at<uchar>(i, j);

			uchar oneMask = 0b11111111 << noBits;
			grayLevel = grayLevel & oneMask; // grayLevel = 0bxx..x00..0

			uchar bitMask = 0b00000000; // grayLevel | bitMask => secretEncoded

			// creates the bitMask to be combined with graylevel
			for (int k = 0; k < noBits; k++) {
				uchar currentBitPostion = 0b00000000;
				currentBitPostion = currentBitPostion | getBit(secret, secretLength, currentBit);
				currentBitPostion = currentBitPostion << k;

				bitMask = bitMask | currentBitPostion;
				currentBit += 1;
			}

			uchar encodedGrayLevel = grayLevel | bitMask;

			dst.at<uchar>(i, j) = encodedGrayLevel;
		}
	}

	return dst;
}


void encode_grayscale_LSB() {
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		// Get the current time [s]
		double t = (double)getTickCount(); 

		Mat src = imread(fname, IMREAD_GRAYSCALE);
		int height = src.rows;
		int width = src.cols;
		Mat dst = Mat(height, width, CV_8UC1);
		


		// Get the current time again and compute the time difference [s]
		t = ((double)getTickCount() - t) / getTickFrequency();

		printf("Time = %.3f [ms]\n", t * 1000);
		imshow("input image", src);
		imshow("encoded image", dst);
		waitKey();
	}
}

void decode_grayscale_LSB() {  
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		// Get the current time [s]
		double t = (double)getTickCount();

		Mat src = imread(fname, IMREAD_GRAYSCALE);
		int height = src.rows;
		int width = src.cols;
		Mat dst = Mat(height, width, CV_8UC1);

		// Get the current time again and compute the time difference [s]

		// Get the current time again and compute the time difference [s]
		// Get the current time again and compute the time difference [s]

		// Get the current time again and compute the time difference [s]
		t = ((double)getTickCount() - t) / getTickFrequency();

		printf("Time = %.3f [ms]\n", t * 1000);
		imshow("input image", src);
		imshow("decoded image", dst);
		waitKey();
	}
}