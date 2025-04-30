// utils.cpp
#include "LSB.h"
#include "stdafx.h"
#include "common.h"
#include "utils.h"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <opencv2/core/utils/logger.hpp>


// TODO() handle different secret formats, input from keyboard, select an image, or select a file
// TODO() the above means to also store some header information


// This function should be called only if the safety checks have been made and the secret is small enough to be encoded.
Mat encode_grayscale(Mat src, std::vector<byte>& secret, int noBits) {

	if (noBits > 8 || noBits < 1) {
		throw std::out_of_range("encode_grayscale: noBits out of index");
	}

	int height = src.rows;
	int width = src.cols;
	Mat dst = src.clone();

	int currentBit = 0;

	for (int i = 0; i < height && currentBit < secret.size(); i++) {
		for (int j = 0; j < width && currentBit < secret.size(); j++) {
			
			uchar grayLevel = src.at<uchar>(i, j);

			uchar oneMask = 0b11111111 << noBits;
			grayLevel = grayLevel & oneMask; // grayLevel = 0bxx..x00..0

			uchar bitMask = 0b00000000; // grayLevel | bitMask => secretEncoded

			// creates the bitMask to be combined with graylevel
			for (int k = 0; k < noBits && currentBit < secret.size(); k++) {
				uchar bit = getBit(secret, currentBit);
				bitMask = bitMask | (bit << k);

				currentBit += 1;
			}

			uchar encodedGrayLevel = grayLevel | bitMask;

			dst.at<uchar>(i, j) = encodedGrayLevel;
		}
	}

	return dst;
}

std::vector<byte> decode_grayscale(const Mat& encoded, int secretBitLength, int noBits) {
	if (noBits < 1 || noBits > 8) {
		throw std::out_of_range("decode_grayscale: noBits must be in [1, 8]");
	}

	std::vector<byte> secret((secretBitLength + 7) / 8, 0);  // allocate enough bytes
	int height = encoded.rows;
	int width = encoded.cols;

	int currentBit = 0;

	for (int i = 0; i < height && currentBit < secretBitLength; i++) {
		for (int j = 0; j < width && currentBit < secretBitLength; j++) {
			uchar grayLevel = encoded.at<uchar>(i, j);

			for (int k = 0; k < noBits && currentBit < secretBitLength; k++, currentBit++) {
				uchar extractedBit = (grayLevel >> k) & 1;

				int byteIndex = currentBit / 8;
				int bitIndex = currentBit % 8;

				secret[byteIndex] = secret[byteIndex] | (extractedBit << bitIndex);
			}
		}
	}

	return secret;
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
		t = ((double)getTickCount() - t) / getTickFrequency();

		printf("Time = %.3f [ms]\n", t * 1000);
		imshow("input image", src);
		imshow("decoded image", dst);
		waitKey();
	}
}