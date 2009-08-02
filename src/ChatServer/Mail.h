/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CHATSERVER_MAIL_H
#define ANH_CHATSERVER_MAIL_H

#include "Utils/typedefs.h"


//======================================================================================================================

class Mail
{
	friend class ChatManager;
	friend class ChatMessageLib;

	public:

		Mail(){ mAttachments = string(BSTRType_Unicode16,2048); }
		~Mail(){}

		uint32	getId(){ return mId; }
		void	setId(uint32 id){mId = id; }

		string	getSender(){ return mSender; }
		void	setSender(const string sender){ mSender = sender; }

		string	getReceiver(){ return mReceiver; }
		void	setReceiver(const string receiver){ mReceiver = receiver; }

		string	getSubject(){ return mSubject; }
		void	setSubject(const string subject){ mSubject = subject; }

		string	getText(){ return mText; }
		void	setText(const string text){ mText = text; }

		uint32	getTime(){ return mTime; }
		void	setTime(uint32 time){ mTime = time; }

		uint8	getStatus(){ return mStatus; }
		void	setStatus(uint8 status){ mStatus = status; }

		string	getAttachments(){ return mAttachments; }
		void	setAttachments(const string attachments){ mAttachments = attachments; }

		uint32	getAttachmentSize(){ return mAttachmentSize; }
		void	setAttachmentSize(uint32 size){ mAttachmentSize = size; }

	private:

		uint32	mId;
		string	mSender;
		string	mReceiver;
		string	mSubject;
		string	mText;
		string	mAttachments;
		uint32	mAttachmentSize;
		uint32	mTime;
		uint8	mStatus;
		char	mAttachmentRaw[2048];
};

#endif

