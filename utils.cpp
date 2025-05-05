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

const std::string SECRET_HEADER_PATH = "\\json\\header.json";
const std::string ENCODED_IMAGES_BASE_PATH = "C:\\Users\\raula\\Desktop\\facultate\\anul 3 sem 2\\Image Processing\\Project\\Dataset\\encoded\\";


std::string extractFileName(const std::string& path) {
	int pos = path.find_last_of("/\\");
	return path.substr(pos + 1);
}

std::string getFileNameWithoutExtension(const std::string& filename) {
	int lastDot = filename.find_last_of('.');
	return filename.substr(0, lastDot);
}

std::string getParentDirectory(const std::string& path) {
	size_t pos = path.find_last_of("/\\");
	return path.substr(0, pos + 1);
}

void processDecodedSecret(const std::vector<byte>& secret, const SecretHeader& header, const std::string& imagePath) {
	int lastSlash = imagePath.find_last_of("/\\");
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


std::vector<byte> generateImageSecret(Mat img) {
	std::vector<byte> secret;


	bool isGrayscale = isImageGrayscale(img);

	if (isGrayscale) {
		for (int i = 0; i < img.rows; ++i) {
			for (int j = 0; j < img.cols; ++j) {
				byte value = img.at<uchar>(i, j);
				secret.push_back(value);
			}
		}
	}
	else {
		for (int i = 0; i < img.rows; ++i) {
			for (int j = 0; j < img.cols; ++j) {

				Vec3b pixel = img.at<Vec3b>(i, j);

				for (int channel = 0; channel < 3; channel++) {
					secret.push_back(pixel[channel]);
				}
			}
		}
	}

	return secret;
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
		std::string secretHeaderPath = getParentDirectory(fname) + SECRET_HEADER_PATH;
		SecretHeader secretHeader = loadSecretHeaderFromFile(secretHeaderPath);

		// choose decoding method
		int encodingMethod;

		// 3 options - LSB, PVD, BSP
		printf(" 0 - Exit\n");
		printf(" 1 - LSB\n");
		scanf("%d", &encodingMethod);
		getchar();
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


		processDecodedSecret(secret, secretHeader, fname);
		imshow("image to decode", imageToDecode);
		waitKey(0);
	}
}

void encodeMessage() {
	// user chooses what secret he wants to embed
	// convert secret from its format into a vector of bytes
	// construct header based on secret information
	// choose what picture to embed the secret into
	// embed the secret into the image
	// save picture at the fname path + /encoded/picture_name/picture
	// at fname path + /encoded/picture_name/json/header.json save header information
	
	// Step 1: Ask user for the secret format
	int secretFormat;
	printf("Select the secret format:\n");
	printf(" 0 - Exit\n");
	printf(" 1 - String\n");
	printf(" 2 - Image\n");
	printf(" 3 - File\n");
	scanf("%d", &secretFormat);
	getchar();
	printf("\n");

	std::vector<byte> secret;
	SecretHeader header;
	std::string secretName;

	// Step 2: Based on the secret format, ask for the secret input
	switch (secretFormat) {

	case 0:
		return;

	// String
	case 1: 
	{
		printf("Enter the secret message (end with an empty line):\n");
		std::string userInput;

		while (true) {
			std::string line;
			std::getline(std::cin, line);
			if (line.empty()) 
				break;
			userInput += line + "\n";
		}

		secret.assign(userInput.begin(), userInput.end());
		header.format = SecretFormat::STRING;
		header.name = "secret.txt";
		header.secretSizeBits = secret.size() * 8;

		break;
	}

	// Image
	case 2:
	{
		printf("Select an image file to embed the secret:\n");
		char secretPath[MAX_PATH];
		while (openFileDlg(secretPath)) {
			Mat secretImage = imread(secretPath, IMREAD_COLOR);

			secret = generateImageSecret(secretImage);
			
			header.format = SecretFormat::IMAGE;
			header.name = extractFileName(secretPath);
			header.secretSizeBits = secret.size() * 8;
			break;
		}

		break;
	}

	// File
	case 3: 
	{
		printf("Select a file to embed the secret:\n");

		char filePath[MAX_PATH];
		if (!openFileDlg(filePath)) {
			printf("Failed to select file.\n");
			return;
		}

		// Read the file into a vector of bytes
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			printf("Failed to open file: %s\n", filePath);
			return;
		}

		secret = std::vector<byte>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		header.format = SecretFormat::FILE;
		header.name = extractFileName(filePath);
		header.secretSizeBits = secret.size() * 8;
		break;
	}

	default:
		printf("Invalid option selected.\n");
		return;

	}

	// Step 3: Read the image to embed the secret into
	Mat imageToEmbed;
	bool isGrayScale = false;

	system("cls");
	printf("Select an image file to embed the secret:\n");

	char imageToEmbedPath[MAX_PATH];
	while (openFileDlg(imageToEmbedPath)) {
		imageToEmbed = imread(imageToEmbedPath, IMREAD_COLOR);
		isGrayScale = isImageGrayscale(imageToEmbed);
		break;
	}
	

	// Step 4: Ask the user for the encoding method (e.g., LSB)
	int encodingMethod;
	system("cls");
	printf(" Select encoding method:\n");
	printf(" 0 - Exit\n");
	printf(" 1 - LSB\n");
	scanf("%d", &encodingMethod);
	getchar();
	printf("\n");


	Mat encodedImage;

	switch (encodingMethod) {
	
	// EXIT
	case 0:
		return;
	
	// LSB
	case 1:
	{
		int bitsUsedPerChannel;
		printf("Bits per channel: ");
		scanf("%d", &bitsUsedPerChannel);
		getchar();
		printf("\n");

		LSBHeader lsbHeader;
		lsbHeader.bitsUsedPerChannel = bitsUsedPerChannel;

		header.encodingMethod = EncodingMethod::LSB;
		header.encodingHeader.lsb = lsbHeader;

		try {
			if (isGrayScale) {
				encodedImage = encode_grayscale_LSB(imageToEmbed, header, secret);
			}
			else {
				encodedImage = encode_color_LSB(imageToEmbed, header, secret);
			}
		}
		catch (const std::out_of_range& e) {
			printf(e.what());
			imshow("couldn't embed the image", imageToEmbed);
			waitKey(0);
			return;
		}
		break;
	}

	default:
		printf("Encoding method: %d not supported\n", encodingMethod);
		break;
	}

	// TODO(): COMPLETE THE HEADER WITH ALL THE INFROMATION

	// Step 5: Save the encoded image and the secret header
	std::string outputFolder = ENCODED_IMAGES_BASE_PATH + getFileNameWithoutExtension(header.name) + "\\";
	std::string jsonFolder = outputFolder + "json\\";

	CreateDirectoryA(outputFolder.c_str(), NULL);
	CreateDirectoryA(jsonFolder.c_str(), NULL);

	if (encodedImage.empty()) {
		printf("The image where the secret should be encoded is empty!\n");
		return;
	}


	std::string imageEncodedName;

	if (header.format == SecretFormat::FILE || header.format == SecretFormat::STRING) {
		imageEncodedName = getFileNameWithoutExtension(header.name) + ".bmp";
	}
	else {
		imageEncodedName = header.name;
	}

	// Save the encoded image
	if (!imwrite(outputFolder + header.name, encodedImage)) {
		printf("Failed to write encoded image.\n");
		return;
	}

	// Save the secret header as JSON
	std::string headerFilePath = jsonFolder + "header.json";
	std::ofstream headerFile(headerFilePath);

	if (!headerFile.is_open()) {
		printf("Failed to write secret header to file.\n");
		return;
	}

	headerFile << serializeSecretHeader(header); 
	headerFile.close();

	printf("Encoded image saved to: %s\n", outputFolder.c_str());
	printf("Secret header saved to: %s\n", headerFilePath.c_str());
	imshow("image with secret", encodedImage);
	imshow("src", imageToEmbed);
	waitKey(0);
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
