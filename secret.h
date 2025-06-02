#pragma once

#include "common.h"
#include "json.hpp"

#include <string>

enum class SecretFormat {
	FILE,
	IMAGE,
	STRING,
	TEST_MODE
};

enum class EncodingMethod {
	LSB,
};

struct LSBHeader {
	uint32_t bitsUsedPerChannel;
};

union EncodingHeader {
	LSBHeader lsb;
};

struct SecretHeader {
	SecretFormat format;		   // format of the secret encoded
	EncodingMethod encodingMethod; // the method used to encode the secret
	std::string name;			   // name contains also the extension (.png, .jpg, .txt, .exe etc)
	unsigned int secretSizeBits;   // secret size in bits
	std::string secretHash;		   // SHA256 hash of the secret to ensure integrity

	EncodingHeader encodingHeader;
};

std::string serializeSecretHeader(const SecretHeader& header);
SecretHeader deserializeSecretHeader(const std::string& jsonStr);