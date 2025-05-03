#pragma once

#include "common.h"
#include <string>

/*
	
	Secrets are going to be encoded into pictures.

	Each image steganography will have 2 types of main methods:
		
		Mat encode(std::vector<byte> secret);
		std::vector<byte> decode(Mat encodedMsg);

	Each secret will me encoded inside an image chosen by the user.
	Each encoded image will be decoded into an array of bytes.

	To handle this 2 standard methods a way to include metadata into
	secret also must be introduced for easier portability of different
	secret formats.

	The secret formats accepted will be: files, images, strings.

	The secret will be encoded in the following format:

	secret_bytes = xxxxxx..xxxx => Secret header
				 = xxxxxx..xxxx =>
				 = xxxxxx..xxxx 	=>
				 = xxxxxx..xxxx 		=> Secret message (size = header.secretSize)
				 = ....				=> 
				 = xxxxxx..xxxx => 
	
	The secret header will be encoded simillar to protobufs formats, meaning:

	Each field will have a number associated with it, such for reference, won't be actually written to it.

	headerSize -> 1
	format -> 2
	name -> 3
	secretSize -> 4

	Before each field actual value, there will be an unsigned integer specifying its size. This way the decoding
	method can know in advance the current field size so it knows how much to read to get to its value.

	Furthermore, this header encoding format is possible because it uses a predefined schema that the program
	is aware of.

	Header encoded example:

	Secretheader {
		headerSize = 200,
		format = FILE,
		name = "test.txt",
		secretSize = 1002453
	}

	The above example will have the following format:

	----------------------------------------------------
	| 32 | 200 | 32 | 0 | 64 | text.txt | 32 | 1002453 | 
	----------------------------------------------------

	As you can see for:
	
		- headerSize => 32 bits to store an uint, 
		- format => 32 bits to store 0 (FILE)
		- name => 64 bits to store "test.txt" (8 ASCII characters => 8 * 1 = 8 bytes)
		- secretSize => 32 bites to store 1002453

*/

enum class SecretFormat {
	FILE,
	IMAGE,
	STRING
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
	unsigned int headerSizeBytes;  // size of header in bytes
	SecretFormat format;		   // format of the secret encoded
	EncodingMethod encodingMethod; // the method used to encode the secret
	std::string name;			   // name contains also the extension (.png, .jpg, .txt, .exe etc)
	unsigned int secretSizeBits;   // secret size in bits

	EncodingHeader encodingHeader;
};

std::vector<byte> encodeSecretHeader(SecretHeader header);
SecretHeader decodeSecretHeader(std::vector<byte>& secret);
