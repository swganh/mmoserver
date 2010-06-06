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

