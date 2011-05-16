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

#include "SendSystemMailMessage.h"
#include "NetworkManager/DispatchClient.h"
#include "MessageLib/MessageLib.h"

SendSystemMailMessage::SendSystemMailMessage() {}
SendSystemMailMessage::~SendSystemMailMessage() {}
//this is basically just a fascade, I had some typdef problems connecting directly to it from some classes.

void SendSystemMailMessage::SendNewbieMailMessage(PlayerObject* player, BString subject, BString bodyDir, BString bodyStr)
{
    if(player && subject.getLength() > 0 && bodyDir.getLength() > 0 && bodyStr.getLength() > 0)
    {
        gMessageLib->sendNewbieMail(player, subject, bodyDir, bodyStr);
    }
}
