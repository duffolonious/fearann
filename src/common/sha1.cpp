/*
 * sha1.cpp
 * Copyright (C) 2006-2008 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <cstdlib>

#include "sha1.h"


//----------------------- SHA1 ----------------------------
uint32_t SHA1::H[5] = { 0, 0, 0, 0, 0 };

inline uint32_t SHA1::leftRotate(uint32_t word, uint8_t bits)
{
	// note that the bits have to be < 32, this shouldn't be a problem and
	// checking for it here would be dumb

	return (word << bits) | (word >> (32-bits));
}

void SHA1::encode(const char* message, string& digest)
{
	// prepare the internal message
	vector<uint8_t> internalMessage;
	while (*message != '\0') {
		internalMessage.push_back(*(message++));
	}

	// reset the seeds
	H[0] = 0x67452301;
	H[1] = 0xEFCDAB89;
	H[2] = 0x98BADCFE;
	H[3] = 0x10325476;
	H[4] = 0xC3D2E1F0;

	// pad the message
	padMessage(internalMessage);

	// process the message (process each block internally)
	processMessage(internalMessage);

	// get readable form of the result
	digest.clear();
	calculateDigest(digest);
}

void SHA1::padMessage(vector<uint8_t>& message)
{
	// mafm: The standard says that we should always pad the message (to
	// form blocks of 512 bits, 64 bytes) even if we already have the
	// desired length initially.  The padding means adding one bit, padding
	// with zeroes until 448 bits (56 bytes), and the other 64 bits to store
	// the size of the original message.  So the implementation goes like
	// this:
	//
	// 1- we save the length of the message, before we manipulate the stream
	//
	// 2- we add 1 bit plus 7 zeroes (our implementation works with bytes,
	// so the original message can't come with "half bytes" anyway).
	// Appending this without checking the size first also ensures that, if
	// the original message is already n*512, we already pad with something.
	//
	// 3- continue padding with zeroes, until we reach 448
	//
	// 4- fill the remaining space with the size of the message (in bits)


	// get message length (in bits) before spoiling it
	uint64_t length = message.size() * 8;

	// aux variables to simulate 1 bit (followed with 7 zeroes) and 8 zeroes
	uint8_t zero = 0x0;
	uint8_t one = (0x1 << 7);

	// add byte with one bit in the beginning
	message.push_back(one);

	// pad with zeroes until 448 (56*8)
	while (message.size()%64 != 56) {
		message.push_back(zero);
	}

	// fill remaining space with the size of the original message
	message.push_back((length >> 56) & 0xff);
	message.push_back((length >> 48) & 0xff);
	message.push_back((length >> 40) & 0xff);
	message.push_back((length >> 32) & 0xff);
	message.push_back((length >> 24) & 0xff);
	message.push_back((length >> 16) & 0xff);
	message.push_back((length >> 8) & 0xff);
	message.push_back((length >> 0) & 0xff);

	// check that the final size is correct (512bits=64Bytes)
	PERM_ASSERT(message.size() % 64 == 0);
}

void SHA1::processMessage(vector<uint8_t>& message)
{
	// mafm: The input for this function has to be a message already padded.
	// Now we process the input message, taking chunks of 512 bits and
	// processing them in turn.

	// check that the input size is correct (512bits = 64Bytes)
	PERM_ASSERT(message.size() % 64 == 0);

	// process each chunk (the callee will only take 64 bytes from the
	// pointer address)
	for (size_t i = 0; i < message.size()/64; ++i) {
		processChunk(&message[i*64]);
	}
}

void SHA1::processChunk(const uint8_t chunk[64])
{
	// mafm: The input for this function is a chunk of 64 bytes (512 bits)
	// ready to be processed, so here's where we apply the core of the
	// algorithm:
	//
	// 1- We need initially an array of 80 uint32_t *BIG ENDIAN* words, of
	// which the first 16 are initialized with the chunk given as parameter,
	// and the 17-80 with values calculated from the initial 16 words
	//
	// 2- Initialize hash value
	//
	// 3- Perform predefined calculations to each range of words
	//
	// 4- Save the data between computation of blocks

	// the structure holding the words to be processed
	uint32_t W[80];
	// fill the first 16 variables with data directly with the chunk, but in
	// *BIG-ENDIAN* order
	for (size_t i = 0; i < 16; ++i) {
		W[i] = chunk[i*4] << 24;
		W[i] |= chunk[i*4 + 1] << 16;
		W[i] |= chunk[i*4 + 2] << 8;
		W[i] |= chunk[i*4 + 3];
	}
	// fill the rest of the variables with values calculated from this one
	for (size_t i = 16; i < 80; ++i) {
		W[i] = leftRotate(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16],
				  1);
	}

	// initialize the hash value (initial magic values given by standard)
	uint32_t A = H[0];
	uint32_t B = H[1];
	uint32_t C = H[2];
	uint32_t D = H[3];
	uint32_t E = H[4];

	// calculate different functions for each range of words
	uint32_t F = 0x0;
	uint32_t K = 0x0;
	for (size_t i = 0; i < 80; ++i) {
		// predefined functions/magic-values for each range
		if (i < 20) {
			F = (B & C) | ((~B) & D);
			K = 0x5A827999;
		} else if (i >= 20 && i < 40) {
			F = B ^ C ^ D;
			K = 0x6ED9EBA1;
		} else if (i >= 40 && i < 60) {
			F = (B & C) | (B & D) | (C & D);
			K = 0x8F1BBCDC;
		} else if (i >= 60) {
			F = B ^ C ^ D;
			K = 0xCA62C1D6;
		}

		// predefined operations with the previous functions
		uint32_t temp = leftRotate(A, 5) + F + E + K + W[i];
		E = D;
		D = C;
		C = leftRotate(B, 30);
		B = A;
		A = temp;
	}

	// save the intermediate calculations
	H[0] += A;
	H[1] += B;
	H[2] += C;
	H[3] += D;
	H[4] += E;
}

void SHA1::calculateDigest(string& digest)
{
	digest.clear();

	// get the 160 bits of storage of H[5] by appending the variables in
	// turn (H0 to H4), having into account that they should be *BIG-ENDIAN*
	for (size_t i = 0; i < 5; ++i) {
		uint8_t b3 = (H[i] >> 24) & 0xff;
		uint8_t b2 = (H[i] >> 16) & 0xff;
		uint8_t b1 = (H[i] >> 8) & 0xff;
		uint8_t b0 = (H[i] >> 0) & 0xff;
		digest.append(StrFmt("%02x", b3));
		digest.append(StrFmt("%02x", b2));
		digest.append(StrFmt("%02x", b1));
		digest.append(StrFmt("%02x", b0));
	}
}

bool SHA1::test()
{
	// predefined messages and results
	const char* testarray[5] = {
		"abc",
		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
		"a",
		"0123456701234567012345670123456701234567012345670123456701234567",
		""
	};
	size_t repeatcount[5] = { 1, 1, 1000000, 10, 1 };
	const char* resultarray[5] = {
		"a9993e364706816aba3e25717850c26c9cd0d89d",
		"84983e441c3bd26ebaae4aa1f95129e5e54670f1",
		"34aa973cd4c4daa4f61eeb2bdbad27316534016f",
		"dea356a2cddd90c7a7ecedc5ebb563934f460452",
		"da39a3ee5e6b4b0d3255bfef95601890afd80709"
	};

	// perform the tests
	for(size_t n = 0; n < 5; ++n) {
		LogNTC("SHA1 test #%zu: %zu loop(s), message: '%s'", n+1, repeatcount[n], testarray[n]);

		string message;
		for (size_t i = 0; i < repeatcount[n]; ++i) {
			message.append(testarray[n]);
		}

		string digest;
		encode(message.c_str(), digest);
		LogNTC("\t'%s'", digest.c_str());
		LogNTC("\t'%s'", resultarray[n]);
		if (digest != resultarray[n]) {
			LogERR("Hash digest doesn't match, aborting test");
			return false;
		}
	}

	return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
