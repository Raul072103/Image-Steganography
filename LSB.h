// utils.h
#pragma once

#include "stdafx.h"

#include <vector>
#include <opencv2/core/utils/logger.hpp>

void encode_LSB();
void decode_LSB();

Mat encode_grayscale_LSB(const Mat& src, SecretHeader header, std::vector<byte>& secret);
Mat encode_color_LSB(const Mat& src, SecretHeader header, std::vector<byte>& secret);

std::vector<byte> decode_grayscale_LSB(const Mat& encoded, int noBits);
std::vector<byte> decode_color_LSB(const Mat& encoded, int noBits);