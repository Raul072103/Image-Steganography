// utils.cpp
#include "stdafx.h"
#include "LSB.h"
#include "stdafx.h"
#include "common.h"
#include "utils.h"
#include "secret.h"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <opencv2/core/utils/logger.hpp>


Mat encode_grayscale_LSB(const Mat &src, SecretHeader header, std::vector<byte>& secret) {

	int noBits = header.encodingHeader.lsb.bitsUsedPerChannel;

	if (noBits > 8 || noBits < 1) {
		throw std::out_of_range("encode_grayscale: noBits out of index");
	}

	int height = src.rows;
	int width = src.cols;
	Mat dst = src.clone();

	int currentBit = header.headerSizeBytes * 8;
	unsigned int secretSize = header.headerSizeBytes * 8 + header.secretSizeBits;


	for (int i = 0; i < height && currentBit < secretSize; i++) {
		for (int j = 0; j < width && currentBit < secretSize; j++) {
			
			uchar grayLevel = src.at<uchar>(i, j);

			uchar oneMask = 0b11111111 << noBits;
			grayLevel = grayLevel & oneMask; // grayLevel = 0bxx..x00..0

			uchar bitMask = 0b00000000; // grayLevel | bitMask => secretEncoded

			// creates the bitMask to be combined with graylevel
			for (int k = 0; k < noBits && currentBit < secretSize; k++) {
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

Mat encode_color_LSB(const Mat& src, SecretHeader header, std::vector<byte>& secret) {

	int noBits = header.encodingHeader.lsb.bitsUsedPerChannel;

	if (noBits > 8 || noBits < 1) {
		throw std::out_of_range("encode_grayscale: noBits out of index");
	}

	int height = src.rows;
	int width = src.cols;
	Mat dst = src.clone();

	int currentBit = header.headerSizeBytes * 8;
	unsigned int secretSize = header.headerSizeBytes * 8 + header.secretSizeBits;


	for (int i = 0; i < height && currentBit < secretSize; i++) {
		for (int j = 0; j < width && currentBit < secretSize; j++) {

			uchar grayLevel = src.at<uchar>(i, j);

			uchar oneMask = 0b11111111 << noBits;
			grayLevel = grayLevel & oneMask; // grayLevel = 0bxx..x00..0

			uchar bitMask = 0b00000000; // grayLevel | bitMask => secretEncoded

			// creates the bitMask to be combined with graylevel
			for (int k = 0; k < noBits && currentBit < secretSize; k++) {
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

std::vector<byte> decode_grayscale_LSB(const Mat& encoded, int noBits) {
	if (noBits < 1 || noBits > 8) {
		throw std::out_of_range("decode_grayscale: noBits must be in [1, 8]");
	}

	std::vector<byte> secret;
	int height = encoded.rows;
	int width = encoded.cols;

	int secretBitsLength;

	int currentBit = 0;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			uchar grayLevel = encoded.at<uchar>(i, j);

			for (int k = 0; k < noBits; k++, currentBit++) {
				uchar extractedBit = (grayLevel >> k) & 1;

				int byteIndex = currentBit / 8;
				int bitIndex = currentBit % 8;

				if (byteIndex >= secret.size()) {
					secret.push_back(0x00);
				}

				secret[byteIndex] = secret[byteIndex] | (extractedBit << bitIndex);
			}
		}
	}

	return secret;
}

std::vector<byte> decode_color_LSB(const Mat& encoded, int noBits) {
	if (noBits < 1 || noBits > 8) {
		throw std::out_of_range("decode_color_LSB: noBits must be in [1, 8]");
	}

	std::vector<byte> secret;
	int height = encoded.rows;
	int width = encoded.cols;

	int currentBit = 0;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			Vec3b pixel = encoded.at<Vec3b>(i, j);

			for (int channel = 0; channel < 3; ++channel) {
				uchar value = pixel[channel];

				for (int k = 0; k < noBits; ++k, ++currentBit) {
					uchar extractedBit = (value >> k) & 1;

					int byteIndex = currentBit / 8;
					int bitIndex = currentBit % 8;

					if (byteIndex >= secret.size()) {
						secret.push_back(0x00);
					}

					secret[byteIndex] |= (extractedBit << bitIndex);
				}
			}
		}
	}

	return secret;
}
