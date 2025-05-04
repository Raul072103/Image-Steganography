#include "stdafx.h"
#include "secret.h"
#include "common.h"

#include <vector>
#include <string>
#include <stdexcept>

// Helper: write 4-byte unsigned int in big-endian
void writeUInt(std::vector<byte>& buffer, unsigned int value) {
	for (int i = 3; i >= 0; --i) {
		buffer.push_back((value >> (i * 8)) & 0xFF);
	}
}

// Helper: read 4-byte unsigned int from offset
unsigned int readUInt(const std::vector<byte>& buffer, unsigned int offset) {
	if (offset + 4 > buffer.size())
		throw std::out_of_range("readUInt: buffer overrun");

	unsigned int value = 0;
	for (int i = 0; i < 4; ++i) {
		value = (value << 8) | buffer[offset++];
	}
	return value;
}

std::vector<byte> encodeSecretHeader(SecretHeader header) {
	std::vector<byte> result;

	// HederSizeBytes
	writeUInt(result, 0);

	// Format 
	writeUInt(result, static_cast<unsigned int>(header.format));

	// Encoding Method
	writeUInt(result, static_cast<unsigned int>(header.encodingMethod));

	// Secret name
	writeUInt(result, header.name.size());
	result.insert(result.end(), std::begin(header.name), std::end(header.name));

	// SecretSizeBits
	writeUInt(result, header.secretSizeBits);

	// Header based on encoding method
	switch (header.encodingMethod) {
	case EncodingMethod::LSB:
		writeUInt(result, header.encodingHeader.lsb.bitsUsedPerChannel);
		break;

	default:
		throw std::invalid_argument("Invalid EncodingMethod");
	}

	// Write the header size
	unsigned int actualSize = result.size();
	result[0] = (actualSize >> 24) & 0xFF;
	result[1] = (actualSize >> 16) & 0xFF;
	result[2] = (actualSize >> 8) & 0xFF;
	result[3] = actualSize & 0xFF;

	return result;
}

SecretHeader decodeSecretHeader(std::vector<byte>& buffer) {
	unsigned int offset = 0;

	// HeaderSizeBytes
	unsigned int headerSize = readUInt(buffer, offset);
	offset += 4;

	// Secret format
	unsigned int formatInt = readUInt(buffer, offset);
	offset += 4;
	if (formatInt > 2)
		throw std::invalid_argument("Invalid SecretFormat");

	// Name size
	unsigned int nameSize = readUInt(buffer, offset);
	offset += 4;

	if (offset + nameSize > buffer.size())
		throw std::out_of_range("decodeSecretHeader: name exceeds buffer");

	std::string name(buffer.begin() + offset, buffer.begin() + offset + nameSize);

	offset += nameSize;

	// SecretSizeBites
	unsigned int secretSize = readUInt(buffer, offset);
	offset += 4;

	// Encoding method
	unsigned int methodInt = readUInt(buffer, offset);
	offset += 4;

	EncodingMethod encodingMethod = static_cast<EncodingMethod>(methodInt);

	// Encoding Header
	EncodingHeader encodingHeader;

	switch (encodingMethod) {
	case EncodingMethod::LSB:
		encodingHeader.lsb.bitsUsedPerChannel = readUInt(buffer, offset);
		offset += 4;
		break;
	default:
		throw std::invalid_argument("Invalid EncodingMethod");
	}

	SecretHeader header;
	header.headerSizeBytes = headerSize;
	header.format = static_cast<SecretFormat>(formatInt);
	header.name = name;
	header.encodingMethod = encodingMethod;
	header.secretSizeBits = secretSize;
	header.encodingHeader = encodingHeader;

	return header;
}
