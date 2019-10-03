
#ifndef MD5_CPP
#define MD5_CPP

#include <iostream>
#include <stdio.h>

/**
 * \class MD5
 * \author Pellegrino Francesco
 * \brief MD5 Class to evaluate signatures
 *

	Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
	rights reserved.

	License to copy and use this software is granted provided that it
	is identified as the "RSA Data Security, Inc. MD5 Message-Digest
	Algorithm" in all material mentioning or referencing this software
	or this function.

	License is also granted to make and use derivative works provided
	that such works are identified as "derived from the RSA Data
	Security, Inc. MD5 Message-Digest Algorithm" in all material
	mentioning or referencing the derived work.

	RSA Data Security, Inc. makes no representations concerning either
	the merchantability of this software or the suitability of this
	software for any particular purpose. It is provided "as is"
	without express or implied warranty of any kind.

	These notices must be retained in any copies of any part of this
	documentation and/or software.

 * \version 1.0.0
 * \date 16/12/2008
 *
 **/

class MD5 {

	public:

		MD5();
		MD5(char* path);						// digest file, close, finalize
		MD5(char* file_buffer,unsigned int file_length);		// digest file, close, finalize

		void update(unsigned char* input, unsigned int input_length);
		void update(FILE *file);
//		void update(FILE *file,unsigned int file_length);
		void finalize();

		// methods to acquire finalized result
		int raw_digest(unsigned char* result);	// digest as a 16-byte binary array
		char* hex_digest();		// digest as a 33-byte ascii-hex string

	private:

		// first, some types:
		typedef unsigned       int uint4; // assumes integer is 4 words long
		typedef unsigned short int uint2; // assumes short integer is 2 words long
		typedef unsigned      char uint1; // assumes char is 1 word long

		// next, the private data:
		uint4 state[4];
		uint4 count[2];     // number of *bits*, mod 2^64
		uint1 buffer[64];   // input buffer
		uint1 digest[16];
		uint1 finalized;

		// last, the private methods, mostly static:
		void init();					// called by all constructors
		void transform(uint1 *buffer);  // does the real update work.  Note that length is implied to be 64.

		static void encode(uint1 *dest, uint4 *src, uint4 length);
		static void decode(uint4 *dest, uint1 *src, uint4 length);
		static void memcpy(uint1 *dest, uint1 *src, uint4 length);
		static void memset(uint1 *start, uint1 val, uint4 length);

		static inline uint4  rotate_left(uint4 x, uint4 n);
		static inline uint4  F(uint4 x, uint4 y, uint4 z);
		static inline uint4  G(uint4 x, uint4 y, uint4 z);
		static inline uint4  H(uint4 x, uint4 y, uint4 z);
		static inline uint4  I(uint4 x, uint4 y, uint4 z);
		static inline void   FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
		static inline void   GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
		static inline void   HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
		static inline void   II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
};

#endif
