/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_COMPCRYPTOR_H
#define ANH_NETWORKMANAGER_COMPCRYPTOR_H

#include "Utils/typedefs.h"


//======================================================================================================================
typedef struct z_stream_s z_stream;


//======================================================================================================================
class CompCryptor
{
public:
    CompCryptor(void);
    ~CompCryptor(void);

    int                               Compress(int8* inData, uint32 inLen, int8* outData, uint32 outLen);
    int                               Decompress(int8* inData, uint32 inLen, int8* outData, uint32 outLen);

    int                               Encrypt(int8* data, uint32 len, uint32 seed);
    int                               Decrypt(int8* data, uint32 len, uint32 seed);

    uint32                            GenerateCRC(int8* data, uint32 len, uint32 seed);

private:
    z_stream*                         mStreamData;
    static const uint32               mCrcTable[256];
};


#endif //ANH_NETWORKMANAGER_COMPCRYPTOR_H



