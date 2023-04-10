/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2022, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/
/** @file  FBXBinaryTokenizer.cpp
 *  @brief Implementation of a fake lexer for binary fbx files -
 *    we emit tokens so the parser needs almost no special handling
 *    for binary files.
 */

#ifndef ASSIMP_BUILD_NO_FBX_IMPORTER

#include "FBXTokenizer.h"
#include "FBXUtil.h"
#include <assimp/defs.h>
#include <assimp/ByteSwapper.h>
#include <assimp/StringUtils.h>

namespace Assimp {
namespace FBX {

// ------------------------------------------------------------------------------------------------
Token::Token(const char* sbegin, const char* send, TokenType type, size_t offset)
    :
    sbegin(sbegin)
    , send(send)
    , type(type)
    , line(offset)
    , column(BINARY_MARKER)
{
}


namespace {

// ------------------------------------------------------------------------------------------------
size_t Offset(const char* begin, const char* cursor) {
    return cursor - begin;
}

// ------------------------------------------------------------------------------------------------
uint32_t ReadWord(const char*, const char*& cursor, const char*) {
    const size_t k_to_read = sizeof( uint32_t );

    uint32_t word;
    ::memcpy(&word, cursor, 4);
    AI_SWAP4(word);

    cursor += k_to_read;

    return word;
}

// ------------------------------------------------------------------------------------------------
uint64_t ReadDoubleWord(const char*, const char*& cursor, const char*) {
    const size_t k_to_read = sizeof(uint64_t);

    uint64_t dword /*= *reinterpret_cast<const uint64_t*>(cursor)*/;
    ::memcpy( &dword, cursor, sizeof( uint64_t ) );
    AI_SWAP8(dword);

    cursor += k_to_read;

    return dword;
}

// ------------------------------------------------------------------------------------------------
uint8_t ReadByte(const char*, const char*& cursor, const char*) {

    uint8_t word;/* = *reinterpret_cast< const uint8_t* >( cursor )*/
    ::memcpy( &word, cursor, sizeof( uint8_t ) );
    ++cursor;

    return word;
}

// ------------------------------------------------------------------------------------------------
unsigned int ReadString(const char*& sbegin_out, const char*& send_out, const char* input,
        const char*& cursor, const char* end, bool long_length = false, bool = false) {

    const uint32_t length = long_length ? ReadWord(input, cursor, end) : ReadByte(input, cursor, end);

    sbegin_out = cursor;
    cursor += length;

    send_out = cursor;

    return length;
}

// ------------------------------------------------------------------------------------------------
void ReadData(const char*& sbegin_out, const char*& send_out, const char* input, const char*& cursor, const char* end) {

    const char type = *cursor;
    sbegin_out = cursor++;

    switch(type)
    {
        // 16 bit int
    case 'Y':
        cursor += 2;
        break;

        // 1 bit bool flag (yes/no)
    case 'C':
        cursor += 1;
        break;

        // 32 bit int
    case 'I':
        // <- fall through

        // float
    case 'F':
        cursor += 4;
        break;

        // double
    case 'D':
        cursor += 8;
        break;

        // 64 bit int
    case 'L':
        cursor += 8;
        break;

        // note: do not write cursor += ReadWord(...cursor) as this would be UB

        // raw binary data
    case 'R':
    {
        const uint32_t length = ReadWord(input, cursor, end);
        cursor += length;
        break;
    }

    case 'b':
        // TODO: what is the 'b' type code? Right now we just skip over it /
        // take the full range we could get
        cursor = end;
        break;

        // array of *
    case 'f':
    case 'd':
    case 'l':
    case 'i':
    case 'c':   {
        ReadWord(input, cursor, end);
        const uint32_t encoding = ReadWord(input, cursor, end);

        const uint32_t comp_len = ReadWord(input, cursor, end);

        // compute length based on type and check against the stored value
        if(encoding == 0) {
            uint32_t stride = 0;
            switch(type)
            {
            case 'f':
            case 'i':
                stride = 4;
                break;

            case 'd':
            case 'l':
                stride = 8;
                break;

            case 'c':
                stride = 1;
                break;
            };
        }
        cursor += comp_len;
        break;
    }

        // string
    case 'S': {
        const char* sb, *se;
        // 0 characters can legally happen in such strings
        ReadString(sb, se, input, cursor, end, true, true);
        break;
    }
    }

    // the type code is contained in the returned range
    send_out = cursor;
}


// ------------------------------------------------------------------------------------------------
bool ReadScope(TokenList& output_tokens, const char* input, const char*& cursor, const char* end, bool const is64bits)
{
    // the first word contains the offset at which this block ends
	const uint64_t end_offset = is64bits ? ReadDoubleWord(input, cursor, end) : ReadWord(input, cursor, end);

    // we may get 0 if reading reached the end of the file -
    // fbx files have a mysterious extra footer which I don't know
    // how to extract any information from, but at least it always
    // starts with a 0.
    if(!end_offset) {
        return false;
    }

    // the second data word contains the number of properties in the scope
	const uint64_t prop_count = is64bits ? ReadDoubleWord(input, cursor, end) : ReadWord(input, cursor, end);

    // the third data word contains the length of the property list
	const uint64_t prop_length = is64bits ? ReadDoubleWord(input, cursor, end) : ReadWord(input, cursor, end);

    // now comes the name of the scope/key
    const char* sbeg, *send;
    ReadString(sbeg, send, input, cursor, end);

    output_tokens.push_back(new_Token(sbeg, send, TokenType_KEY, Offset(input, cursor) ));

    // now come the individual properties
    const char* begin_cursor = cursor;

    for (unsigned int i = 0; i < prop_count; ++i) {
        ReadData(sbeg, send, input, cursor, begin_cursor + prop_length);

        output_tokens.push_back(new_Token(sbeg, send, TokenType_DATA, Offset(input, cursor) ));

        if(i != prop_count-1) {
            output_tokens.push_back(new_Token(cursor, cursor + 1, TokenType_COMMA, Offset(input, cursor) ));
        }
    }

    // at the end of each nested block, there is a NUL record to indicate
    // that the sub-scope exists (i.e. to distinguish between P: and P : {})
    // this NUL record is 13 bytes long on 32 bit version and 25 bytes long on 64 bit.
	const size_t sentinel_block_length = is64bits ? (sizeof(uint64_t)* 3 + 1) : (sizeof(uint32_t)* 3 + 1);

    if (Offset(input, cursor) < end_offset) {

        output_tokens.push_back(new_Token(cursor, cursor + 1, TokenType_OPEN_BRACKET, Offset(input, cursor) ));

        // XXX this is vulnerable to stack overflowing ..
        while(Offset(input, cursor) < end_offset - sentinel_block_length) {
			ReadScope(output_tokens, input, cursor, input + end_offset - sentinel_block_length, is64bits);
        }
        output_tokens.push_back(new_Token(cursor, cursor + 1, TokenType_CLOSE_BRACKET, Offset(input, cursor) ));

        cursor += sentinel_block_length;
    }

    return true;
}

} // anonymous namespace

// ------------------------------------------------------------------------------------------------
// TODO: Test FBX Binary files newer than the 7500 version to check if the 64 bits address behaviour is consistent
void TokenizeBinary(TokenList& output_tokens, const char* input, size_t length)
{
    const char* cursor = input + 18;
	/*Result ignored*/ ReadByte(input, cursor, input + length);
	/*Result ignored*/ ReadByte(input, cursor, input + length);
	/*Result ignored*/ ReadByte(input, cursor, input + length);
	/*Result ignored*/ ReadByte(input, cursor, input + length);
	/*Result ignored*/ ReadByte(input, cursor, input + length);
	const uint32_t version = ReadWord(input, cursor, input + length);

	const bool is64bits = version >= 7500;
    const char *end = input + length;

    while (cursor < end ) {
        if (!ReadScope(output_tokens, input, cursor, input + length, is64bits)) {
            break;
        }
    }
}

} // !FBX
} // !Assimp

#endif
