/*
	MI Control Utility — v1.2.5 [public version]
	Copyright (C) 2025 ALXR aka loginsin
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <array>

typedef struct {
    ULONG         i[2];
    ULONG         buf[4];
    unsigned char in[64];
    unsigned char digest[16];
} MD5_CTX;

extern "C" __declspec(dllimport) void WINAPI MD5Init(MD5_CTX*);
extern "C" __declspec(dllimport) void WINAPI MD5Update(MD5_CTX*, const BYTE* input, unsigned int inlen);
extern "C" __declspec(dllimport) void WINAPI MD5Final(MD5_CTX*);

using MD5 = std::array<BYTE, 16>;

MD5 MD5Calc(const BYTE * pData, size_t uLength);
MD5 MD5Calc(const std::vector<BYTE>& data);