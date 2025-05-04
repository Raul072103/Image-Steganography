#pragma once

#include "common.h"
#include "stdafx.h"

#include <iostream>
#include <vector>
#include <opencv2/core/utils/logger.hpp>

// Application flow functions
void decodeMessage();
void encodeMessage();

// Encoding utility functions
std::vector<byte> convertSecretToBytes();

// Decoding utility functions

// Utility functions
bool getBit(std::vector<byte>& secret, int n);
bool isImageGrayscale(Mat& src);
