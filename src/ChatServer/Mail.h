/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_CHATSERVER_MAIL_H
#define ANH_CHATSERVER_MAIL_H

#include "Utils/typedefs.h"
#include "Utils/bstring.h"


//======================================================================================================================

class Mail
{
    friend class ChatManager;
    friend class ChatMessageLib;

public:

    Mail() {
        mAttachments = BString(BSTRType_Unicode16,2048);
    }
    ~Mail() {}

    uint32	getId() {
        return mId;
    }
    void	setId(uint32 id) {
        mId = id;
    }

    BString	getSender() {
        return mSender;
    }
    void	setSender(const BString sender) {
        mSender = sender;
    }

    BString	getReceiver() {
        return mReceiver;
    }
    void	setReceiver(const BString receiver) {
        mReceiver = receiver;
    }

    BString	getSubject() {
        return mSubject;
    }
    void	setSubject(const BString subject) {
        mSubject = subject;
    }

    BString	getText() {
        return mText;
    }
    void	setText(const BString text) {
        mText = text;
    }

    uint32	getTime() {
        return mTime;
    }
    void	setTime(uint32 time) {
        mTime = time;
    }

    uint8	getStatus() {
        return mStatus;
    }
    void	setStatus(uint8 status) {
        mStatus = status;
    }

    BString	getAttachments() {
        return mAttachments;
    }
    void	setAttachments(const BString attachments) {
        mAttachments = attachments;
    }

    uint32	getAttachmentSize() {
        return mAttachmentSize;
    }
    void	setAttachmentSize(uint32 size) {
        mAttachmentSize = size;
    }

private:

    uint32	mId;
    BString	mSender;
    BString	mReceiver;
    BString	mSubject;
    BString	mText;
    BString	mAttachments;
    uint32	mAttachmentSize;
    uint32	mTime;
    uint8	mStatus;
    char	mAttachmentRaw[2048];
};

#endif

