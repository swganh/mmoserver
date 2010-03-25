/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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

  void                              Startup(void);
  void                              Shutdown(void);

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



