// utils.h
#pragma once

#include "stdafx.h"
#include "secret.h"

#include <vector>
#include <iostream>
#include <opencv2/core/utils/logger.hpp>

Mat encode_grayscale_LSB(const Mat& src, SecretHeader header, std::vector<byte>& secret);
Mat encode_color_LSB(const Mat& src, SecretHeader header, std::vector<byte>& secret);

std::vector<byte> decode_grayscale_LSB(const Mat& encoded, SecretHeader header);
std::vector<byte> decode_color_LSB(const Mat& encoded, SecretHeader header);