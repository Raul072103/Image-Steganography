#include "stdafx.h"
#include "utils.h"
#include "secret.h"
#include "common.h"
#include "LSB.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <windows.h>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/opencv.hpp>

const std::string SECRET_HEADER_PATH = "/json/header.json";

void processDecodedSecret(const std::vector<byte>& secret, const SecretHeader& header, const std::string& imagePath) {
	size_t lastSlash = imagePath.find_last_of("/\\");
	std::string baseFolder = imagePath.substr(0, lastSlash);
	std::string outputFolder = baseFolder + "\\decode";

	// Ensure output folder exists
	CreateDirectoryA(outputFolder.c_str(), NULL);

	std::string outputPath = outputFolder + "\\" + header.name;

	switch (header.format) {
	case SecretFormat::STRING: {
		std::string message(secret.begin(), secret.end());
		printf("Decoded string:\n%s\n", message.c_str());
		break;
	}

	case SecretFormat::FILE: {
		std::ofstream outFile(outputPath, std::ios::binary);
		if (!outFile.is_open()) {
			printf("Failed to write decoded file to: %s\n", outputPath.c_str());
			return;
		}
		outFile.write(reinterpret_cast<const char*>(secret.data()), secret.size());
		outFile.close();
		printf("Decoded file saved to: %s\n", outputPath.c_str());
		break;
	}

	case SecretFormat::IMAGE: {
		Mat decodedImage = imdecode(secret, cv::IMREAD_UNCHANGED);
		if (decodedImage.empty()) {
			printf("Failed to decode image from secret data.\n");
			return;
		}

		if (!imwrite(outputPath, decodedImage)) {
			printf("Failed to write decoded image to: %s\n", outputPath.c_str());
			return;
		}

		printf("Decoded image saved to: %s\n", outputPath.c_str());
		break;
	}

	default:
		printf("Unknown secret format.\n");
		break;
	}
}


SecretHeader loadSecretHeaderFromFile(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open JSON file: " + filePath);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string jsonStr = buffer.str();

	return deserializeSecretHeader(jsonStr);
}

bool isImageGrayscale(Mat& img) {

	if (img.channels() == 3) {
		for (int i = 0; i < img.rows; ++i) {
			for (int j = 0; j < img.cols; ++j) {
				Vec3b pixel = img.at<Vec3b>(i, j);

				if (pixel[0] != pixel[1] || pixel[1] != pixel[2]) {
					return false;
				}
			}
		}
		return true;
	}

	return false;
}

void decodeMessage() {
	Mat imageToDecode;
	bool isGrayScale;

	std::vector<byte> secret;

	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		// get image
		imageToDecode = imread(fname, IMREAD_COLOR);
		isGrayScale = isImageGrayscale(imageToDecode);

		// get secret header - convention the secret header is stored on the same path as the picture + /json/header.json
		std::string secretHeaderPath = fname + SECRET_HEADER_PATH;
		SecretHeader secretHeader = loadSecretHeaderFromFile(secretHeaderPath);

		// choose decoding method
		int encodingMethod;

		// 3 options - LSB, PVD, BSP
		printf(" 0 - Exit\n");
		printf(" 1 - LSB\n");
		scanf("%d", &encodingMethod);
		printf("\n");

		switch (encodingMethod)
		{
		case 0:
			break;

		// LSB
		case 1:
			if (isGrayScale) {
				secret = decode_grayscale_LSB(imageToDecode, secretHeader);
			}
			else {
				secret = decode_color_LSB(imageToDecode, secretHeader);
			}

			break;

		default:
			printf("You can only choose encode or decode\n");
			break;
		}


		// TODO(): construct the original secret based on the secret header
		processDecodedSecret(secret, secretHeader, fname);
		waitKey();
		imshow("image to decode", imageToDecode);
	}
}

bool getBit(std::vector<byte>& secret, int n) {

	// 1 byte = 8 bits => total = secretLength * 8
	if (n < 0 || n >= secret.size() * 8) {
		throw std::out_of_range("getBit: bit index out of range");
	}
	
	unsigned int bytePostion = n / 8;
	unsigned int bitPosition = n % 8;

	byte selectedByte = secret[bytePostion];

	byte bitMask = 0b00000001 << bitPosition;

	// shift selected bit on first position so we can detect if it is a 0 or 1
	return ((selectedByte & bitMask) >> bitPosition) == 1;
}

std::vector<byte> convertSecretToBytes() {
	return std::vector<byte>();
}
