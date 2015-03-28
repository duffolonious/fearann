/*
 * sha1.h
 * Copyright (C) 2006 by Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
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

#ifndef __FEARANN_COMMON_SHA1_H__
#define __FEARANN_COMMON_SHA1_H__

#include <vector>
#include <string>


/** This class implements SHA1 (US Secure Hash Algorithm 1, see in example
 * RFC3174), which is basically useful to send a hash of our passwords to the
 * server, so the password itself remains secret.  The implementation may not be
 * complete according to other SHA1 implementations (it has only the necessary
 * parts that we might use) nor is meant to be completely secure (it's not
 * formally verified and may have bugs that lead to security leaks not present
 * in the algorithm itself).
 *
 * Anyway, this code produces the correct hash of some of the test messages
 * given by the standard itself (given as reference to check that your
 * implementation is correct), so we can be confident to say that it's a
 * compliant and correct (but not verified) SHA1 implementation.
 */
class SHA1
{
public:
	/** Get the hash of the given message */
	static void encode(const char* message, std::string& digest);

	/** Test the correctness, by testing the output of predefined strings
	 * against the previously known results.  Left as public because we
	 * might want to check that this is OK when starting the application or
	 * so, to check for problems in some architectures with word sizes and
	 * so on.  */
	static bool test();

private:
	/// Magic numbers defined initially and saved between computations
	static uint32_t H[5];

	/** Shortcut for one operation needed in calculations. */
	static inline uint32_t leftRotate(uint32_t word, uint8_t bits);

	/** Pad the message */
	static void padMessage(std::vector<uint8_t>& message);
	/** Process the message, taking 512bit-size blocks in turn and process
	 * them. */
	static void processMessage(std::vector<uint8_t>& message);
	/** Process chunk of 512bit-size with the core of the algorithm. */
	static void processChunk(const uint8_t chunk[64]);
	/** Calculate digest. */
	static void calculateDigest(std::string& digest);
};

#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 8 ***
// indent-tabs-mode: t ***
// fill-column: 80 ***
// End: ***
// ex: shiftwidth=2 tabstop=8
