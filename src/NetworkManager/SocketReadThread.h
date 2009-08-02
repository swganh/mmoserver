/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_SOCKETREADTHREAD_H
#define ANH_NETWORKMANAGER_SOCKETREADTHREAD_H

#include "zthread/ZThread.h"
#include "Utils/typedefs.h"
#include <list>
#include <map>
	
//======================================================================================================================

class SocketWriteThread;
class PacketFactory;
class SessionFactory;
class MessageFactory;
class CompCryptor;
class Session;
class Service;
class Packet;

typedef std::list<Session*>			SessionList;
typedef std::map<uint64,Session*>	AddressSessionMap;    //Hash based on IP and port we might consider adding other stuff with 
														   //with a loginmanager to allow multiple clients per IP if so wihed
                                                                  
                                                                  
typedef unsigned int SOCKET;                                      

class NewConnection
{
	public:
	  int8              mAddress[256];
	  uint16            mPort;
	  Session*          mSession;
};


//======================================================================================================================

class SocketReadThread
{
	public:
									SocketReadThread(void);

	  void                          Startup(SOCKET socket, SocketWriteThread* writeThread, Service* service,uint32 mfHeapSize, bool serverservice);
	  void                          Shutdown(void);
	  virtual void                 run();

	  void                          NewOutgoingConnection(int8* address, uint16 port);
	  void                          RemoveAndDestroySession(Session* session);

	  NewConnection*                getNewConnectionInfo(void)  { return &mNewConnection; };
	  bool                          getIsRunning(void)          { return mIsRunning; }
	  void							requestExit(){ mExit = true; }

	protected:

	  void                          _startup(void);
	  void                          _shutdown(void);

	  Packet*                       mReceivePacket;
	  Packet*                       mDecompressPacket;

	  uint16						mMessageMaxSize;
	  SocketWriteThread*            mSocketWriteThread;
	  SessionFactory*               mSessionFactory;
	  PacketFactory*                mPacketFactory;
	  MessageFactory*               mMessageFactory;
	  CompCryptor*                  mCompCryptor;
	  NewConnection                 mNewConnection;

	  SOCKET                        mSocket;

	  bool							mIsRunning;

	  uint32						mSessionResendWindowSize;
	  ZThread::Thread*				mThread;
	  ZThread::Mutex                mSocketReadMutex;
	  AddressSessionMap             mAddressSessionMap;
	  bool							mExit;
};

//======================================================================================================================

class SocketReadThreadRunnable : public ZThread::Runnable
{
	public:
		SocketReadThreadRunnable(SocketReadThread* r){ mReadThread = r; }
		~SocketReadThreadRunnable(){}

	virtual void run(){ mReadThread->run(); }

	SocketReadThread* mReadThread;
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_SOCKETREADTHREAD_H




