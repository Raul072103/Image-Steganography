#include "stdafx.h"
#include "common.h"
#include "utils.h"
#include "secret.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/opencv.hpp>

const std::vector<std::pair<int, int>> pvd_ranges = {
    {0, 7}, {8, 15}, {16, 31}, {32, 63}, {64, 127}, {128, 255}
};

int get_range_index(int diff_abs) {
    for (int i = 0; i < pvd_ranges.size(); ++i) {
        if (diff_abs >= pvd_ranges[i].first && diff_abs <= pvd_ranges[i].second)
            return i;
    }
    return -1;
}

void encodeChannel(Mat& channel, size_t& bit_idx, std::vector<byte>& secret, const SecretHeader& header) {
    for (int i = 0; i < channel.rows; ++i) {
        for (int j = 0; j < channel.cols - 1; j += 2) {
            uchar g1 = channel.at<uchar>(i, j);
            uchar g2 = channel.at<uchar>(i, j + 1);
            int d = g2 - g1;
            int d_abs = std::abs(d);

            int ri = get_range_index(d_abs);
            if (ri == -1) continue;

            int li = pvd_ranges[ri].first;
            int ui = pvd_ranges[ri].second;
            int wi = ui - li + 1;
            int n = static_cast<int>(std::floor(std::log2(wi)));

            if (bit_idx + n > header.secretSizeBits) return;

            int bk = 0;
            for (int b = 0; b < n; ++b) {
                bk = (bk << 1) | getBit(secret, bit_idx);
                ++bit_idx;
            }

            int d_prime = (d >= 0) ? li + bk : -(li + bk);
            float m = (d_prime - d) / 2.0f;
            
            /*
            int new_g1;
            int new_g2;

            int floor_m = static_cast<int>(floor(m));
            int ceil_m = static_cast<int>(ceil(m));

            if (d_prime % 2 != 0) {
                new_g1 = g1 - ceil_m;
                new_g2 = g2 + floor_m;
            }
            else {
                new_g1 = g1 - floor_m;
                new_g2 = g2 + ceil_m;
            }
            */

            int m = (d_prime - d) / 2;
            int new_g1 = g1 - m;
            int new_g2 = g2 + m;

            if (new_g1 < 0 || new_g1 > 255 || new_g2 < 0 || new_g2 > 255) {
                bit_idx -= n;
                continue;
            }

            channel.at<uchar>(i, j) = static_cast<uchar>(new_g1);
            channel.at<uchar>(i, j + 1) = static_cast<uchar>(new_g2);
        }
    }
}

void decodeChannel(const Mat& channel, std::vector<bool>& bits, size_t total_bits) {
    for (int i = 0; i < channel.rows; ++i) {
        for (int j = 0; j < channel.cols - 1; j += 2) {
            uchar g1 = channel.at<uchar>(i, j);
            uchar g2 = channel.at<uchar>(i, j + 1);
            int d = g2 - g1;
            int d_abs = std::abs(d);

            int ri = get_range_index(d_abs);
            if (ri == -1) {
                continue;
            }

            int li = pvd_ranges[ri].first;
            int ui = pvd_ranges[ri].second;
            int wi = ui - li + 1;
            int n = static_cast<int>(std::floor(std::log2(wi)));

            if (bits.size() + n > total_bits) {
                return;
            }

            int bk = d_abs - li;

            for (int b = n - 1; b >= 0; --b) {
                bits.push_back((bk >> b) & 1);
            }
        }
    }
}

Mat encode_PVD(const Mat& src, SecretHeader header, std::vector<byte>& secret) {
    Mat encodedMat = src.clone();
    size_t bit_idx = 0;

    if (isImageGrayscale(src)) {
        encodeChannel(encodedMat, bit_idx, secret, header);
    }
    else {
        std::vector<Mat> channels(3);
        split(encodedMat, channels);

        encodeChannel(channels[0], bit_idx, secret, header); // B
        encodeChannel(channels[1], bit_idx, secret, header); // G
        encodeChannel(channels[2], bit_idx, secret, header); // R

        merge(channels, encodedMat);
    }

    return encodedMat;
}


std::vector<byte> append_bits(const std::vector<bool>& bits) {
    std::vector<byte> result;
    byte current_byte = 0;
    int bit_count = 0;

    for (bool bit : bits) {
        current_byte = (current_byte << 1) | static_cast<byte>(bit);
        bit_count++;

        if (bit_count == 8) {
            result.push_back(current_byte);
            current_byte = 0;
            bit_count = 0;
        }
    }

    if (bit_count > 0) {
        current_byte <<= (8 - bit_count);  // pad with zeros
        result.push_back(current_byte);
    }

    return result;
}

std::vector<byte> decode_PVD(const Mat& encoded, SecretHeader header) {
    std::vector<bool> bits;
    size_t total_bits = header.secretSizeBits;

    if (isImageGrayscale(encoded)) {
        decodeChannel(encoded, bits, total_bits);
    }
    else {
        std::vector<Mat> channels(3);
        split(encoded, channels);

        decodeChannel(channels[0], bits, total_bits);
        decodeChannel(channels[1], bits, total_bits);
        decodeChannel(channels[2], bits, total_bits);
    }

    return append_bits(bits);
}
