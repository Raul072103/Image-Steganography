#pragma once
// utils.h
#pragma once

#include "stdafx.h"
#include "secret.h"

#include <vector>
#include <iostream>
#include <opencv2/core/utils/logger.hpp>

Mat encode_PVD(const Mat& src, SecretHeader header, std::vector<byte>& secret);

std::vector<byte> decode_PVD(const Mat& encoded, SecretHeader header);