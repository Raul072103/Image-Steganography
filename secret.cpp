#include "stdafx.h"
#include "secret.h"
#include "common.h"
#include "json.hpp"

#include <vector>
#include <string>
#include <stdexcept>

using json = nlohmann::json;

std::string serializeSecretHeader(const SecretHeader& header) {
	json j;
	j["format"] = static_cast<int>(header.format);
	j["encodingMethod"] = static_cast<int>(header.encodingMethod);
	j["name"] = header.name;
	j["secretSizeBits"] = header.secretSizeBits;
	j["secretHash"] = header.secretHash;

	// Serialize method-specific encoding header based on the encoding method.
	if (header.encodingMethod == EncodingMethod::LSB) {
		j["encodingHeader"] = {
			{"bitsUsedPerChannel", header.encodingHeader.lsb.bitsUsedPerChannel}
		};
	}

	return j.dump();  // Convert JSON object to string
}

SecretHeader deserializeSecretHeader(const std::string& jsonStr) {
	json j = json::parse(jsonStr);

	SecretHeader header;
	header.format = static_cast<SecretFormat>(j["format"].get<int>());
	header.encodingMethod = static_cast<EncodingMethod>(j["encodingMethod"].get<int>());
	header.name = j["name"].get<std::string>();
	header.secretSizeBits = j["secretSizeBits"].get<unsigned int>();
	header.secretHash = j["secretHash"].get<std::string>();

	// Deserialize method-specific header
	if (header.encodingMethod == EncodingMethod::LSB) {
		header.encodingHeader.lsb.bitsUsedPerChannel = j["encodingHeader"]["bitsUsedPerChannel"].get<uint32_t>();
	}

	return header;
}