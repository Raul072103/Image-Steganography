#include "stdafx.h"
#include "common.h"
#include "utils.h"
#include "secret.h"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <opencv2/core/utils/logger.hpp>

const Mat QUANT_TABLE = (Mat_<float>(8, 8) <<
	16, 11, 10, 16, 24, 40, 51, 61,
	12, 12, 14, 19, 26, 58, 60, 55,
	14, 13, 16, 24, 40, 57, 69, 56,
	14, 17, 22, 29, 51, 87, 80, 62,
	18, 22, 37, 56, 68, 109, 103, 77,
	24, 36, 55, 64, 81, 104, 113, 92,
	49, 64, 78, 87, 103, 121, 120, 101,
	72, 92, 95, 98, 112, 100, 103, 99
);

const int zigzagIndex[64][2] = {
	{0,0},{0,1},{1,0},{2,0},{1,1},{0,2},{0,3},{1,2},
	{2,1},{3,0},{4,0},{3,1},{2,2},{1,3},{0,4},{0,5},
	{1,4},{2,3},{3,2},{4,1},{5,0},{6,0},{5,1},{4,2},
	{3,3},{2,4},{1,5},{0,6},{0,7},{1,6},{2,5},{3,4},
	{4,3},{5,2},{6,1},{7,0},{7,1},{6,2},{5,3},{4,4},
	{3,5},{2,6},{1,7},{2,7},{3,6},{4,5},{5,4},{6,3},
	{7,2},{7,3},{6,4},{5,5},{4,6},{3,7},{4,7},{5,6},
	{6,5},{7,4},{7,5},{6,6},{5,7},{6,7},{7,6},{7,7}
};


Mat cropTo8x8(const Mat& img) {
	int croppedWidth = img.cols - (img.cols % 8);
	int croppedHeight = img.rows - (img.rows % 8);

	Rect roi(0, 0, croppedWidth, croppedHeight);

	return img(roi).clone();
}

float alpha(int x) {
	return (x == 0) ? (1.0f / sqrt(2.0f)) : 1.0f;
}

std::vector<int> zigzag(const Mat& block) {
	std::vector<int> result(64);
	for (int i = 0; i < 64; ++i) {
		int x = zigzagIndex[i][0];
		int y = zigzagIndex[i][1];
		result[i] = block.at<int>(x, y);
	}
	return result;
}

Mat inverseZigzag(const std::vector<int>& vec) {
	Mat block(8, 8, CV_32S);
	for (int i = 0; i < 64; ++i) {
		int x = zigzagIndex[i][0];
		int y = zigzagIndex[i][1];
		block.at<int>(x, y) = vec[i];
	}
	return block;
}

Mat encode_DCT(const Mat& src, SecretHeader header, std::vector<byte>& secret) {
	// cropping step
	Mat croppedImg = cropTo8x8(src);

	// uchar to float
	Mat imgFloat;
	croppedImg.convertTo(imgFloat, CV_32F);

	// float to 2YCrCb
	Mat imgYCrCb;
	cvtColor(imgFloat, imgYCrCb, COLOR_BGR2YCrCb);

	std::vector<Mat> channels;
	split(imgYCrCb, channels);

	Mat imgToApply = channels[0];

	int height = imgToApply.rows;
	int width = imgToApply.cols;

	std::vector<std::vector<int>> sortedQuantVectors;

	// split into 8x8
	// forward DCT stage
	// quantization stage

	for (int i = 0; i < height; i += 8) {
		for (int j = 0; j < width; j += 8) {
			Mat currMat = Mat(8, 8, CV_32S);

			for (int u = 0; u < 8; ++u) {
				for (int v = 0; v < 8; ++v) {
					float sum = 0.0f;

					for (int x = 0; x < 8; ++x) {
						for (int y = 0; y < 8; ++y) {

							float pixel = imgToApply.at<float>(i+x, j+y);
							float cu = cos(((2.0f * x + 1) * u * PI) / 16.0f);
							float cv = cos(((2.0f * y + 1) * v * PI) / 16.0f);

							sum += pixel * cu * cv;
						}
					}
					float dctResult = 0.25f * alpha(u) * alpha(v) * sum;
					float quantizedDCTResult = int(std::round(dctResult / QUANT_TABLE.at<float>(u, v)));

					currMat.at<int>(u, v) = quantizedDCTResult;
				}
			}
			// sort DCT, basically just zigZag
			sortedQuantVectors.push_back(zigzag(currMat));
		}
	}


	// embed data in luminane layer
	std::vector<std::vector<int>> embeddedVec;
	int bitPos = 0;

	for (int i = 0; i < sortedQuantVectors.size(); i++) {
		std::vector<int> currVec = sortedQuantVectors[i];

		// skip the first coefficient, apparently it distorts the image the most
		for (int j = 1; j < currVec.size(); j++) {
			int coeff = currVec[j];

			if (bitPos < secret.size() && coeff > 1) {
				currVec[j] = (currVec[j] & ~1) | getBit(secret, bitPos);
				bitPos += 1;
			}
		}

		embeddedVec.push_back(currVec);
	}

	// inverse zigzag
	std::vector<Mat> inverseMat;

	for (int i = 0; i < embeddedVec.size(); i++) {
		std::vector<int> currVec = embeddedVec[i];
		Mat currMat = inverseZigzag(currVec);

		inverseMat.push_back(currMat);
	}

	// dequantization process + inverse dct
	std::vector<Mat> idctMat;
	for (int i = 0; i < inverseMat.size(); i++) {
		Mat currMat = inverseMat[i];

		Mat currMatFloat;
		currMat.convertTo(currMatFloat, CV_32F);

		Mat dequantized;
		multiply(currMatFloat, QUANT_TABLE, dequantized);

		Mat idctResult;
		idct(dequantized, idctResult);

		idctMat.push_back(idctResult);
	}

	Mat reconstructedImg = Mat::zeros(height, width, CV_32F);

	int blockIdx = 0;
	for (int i = 0; i < height; i += 8) {
		for (int j = 0; j < width; j += 8) {
			Mat block = idctMat[blockIdx];
			blockIdx += 1;

			for (int x = 0; x < 8; ++x) {
				for (int y = 0; y < 8; ++y) {
					reconstructedImg.at<float>(i + x, j + y) = block.at<float>(x, y);
				}
			}
		}
	}
	channels[0] = reconstructedImg;

	Mat mergedYCrCb;
	merge(channels, mergedYCrCb);

	Mat finalRGB;
	cvtColor(mergedYCrCb, finalRGB, COLOR_YCrCb2BGR);

	Mat ucharImage;
	finalRGB.convertTo(ucharImage, CV_8UC3, 1.0, 0);

	imshow("hello", ucharImage);
	waitKey(0);

	return ucharImage;
}

std::vector<byte> decode_DCT(const Mat& encoded, SecretHeader header) {
	Mat imgFloat;
	encoded.convertTo(imgFloat, CV_32F);

	Mat imgYCrCb;
	cvtColor(imgFloat, imgYCrCb, COLOR_BGR2YCrCb);

	std::vector<Mat> channels;
	split(imgYCrCb, channels);

	Mat imgToApply = channels[0];  // luminance channel

	int height = imgToApply.rows;
	int width = imgToApply.cols;

	std::vector<std::vector<int>> quantVectors;

	// Extract 8x8 blocks, DCT, quantize, then read bits from LSBs
	for (int i = 0; i < height; i += 8) {
		for (int j = 0; j < width; j += 8) {
			Mat block = imgToApply(Rect(j, i, 8, 8));

			Mat dctBlock;
			dct(block, dctBlock);

			Mat quantized(8, 8, CV_32FC1);

			for (int x = 0; x < quantized.rows; ++x) {
				for (int y = 0; y < quantized.cols; ++y) {
					quantized.at<float>(x, y) = std::round(dctBlock.at<float>(x, y) / QUANT_TABLE.at<float>(x, y));
				}
			}

			Mat quantizedInt;
			quantized.convertTo(quantizedInt, CV_32S);
			quantVectors.push_back(zigzag(quantizedInt));
		}
	}

	std::vector<byte> secretBits;

	int totalBits = header.secretSizeBits; 
	int bitCount = 0;

	for (const auto& vec : quantVectors) {
		for (int k = 1; k < (int)vec.size(); ++k) {
			if (vec[k] > 1) {
				int bit = vec[k] & 1;
				secretBits.push_back(bit);
				bitCount++;
				
				if (bitCount >= totalBits) {
					break;
				}
			}
		}

		if (bitCount >= totalBits) {
			break;
		}
	}

	// Convert bits to bytes
	std::vector<byte> decodedMessage;

	for (int i = 0; i < header.secretSizeBits; i += 8) {
		byte val = 0;
		for (int b = 0; b < 8; ++b) {
			val |= (secretBits[i + b] << (7 - b));
		}

		decodedMessage.push_back(val);
	}

	return decodedMessage;
}