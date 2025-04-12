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

#include "precomp.h"

#pragma comment(lib, "cryptdll.lib")

MD5 MD5Calc(const BYTE* pData, size_t uLength)
{
    MD5 result;

    MD5_CTX context;
    MD5Init(&context);
    MD5Update(&context, pData, static_cast<unsigned int>(uLength));
    MD5Final(&context);
    CopyMemory(result.data(), context.digest, sizeof(context.digest));
    return result;
}

MD5 MD5Calc(const std::vector<BYTE>& data)
{
    return MD5Calc(data.data(), data.size());
}