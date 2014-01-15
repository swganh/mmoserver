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

#ifndef ANH_ZONESERVER_CSROPCODES_H
#define ANH_ZONESERVER_CSROPCODES_H

enum csr_opcodes
{
    //C->S
    opSearchKnowledgeBaseMessage			= 0x962E8B9B,
    opRequestCategoriesMessage				= 0xF898E25F,
    opNewTicketActivityMessage				= 0x274F4E78,
    opGetTicketsMessage						= 0xC9A5F98D,
    opGetCommentsMessage					= 0x270A9EC5,
    opGetArticleMessage						= 0x5E7B4846,
    opCreateTicketMessage					= 0x40E64DAC,
    opCancelTicketMessage					= 0x638EF431,
    opAppendCommentMessage					= 0x0AC49644,

    //S->C
    opSearchKnowledgeBaseResponseMessage	= 0x7CBC8F67,
    opRequestCategoriesResponseMessage		= 0x61148FD4,
    opNewTicketActivityResponseMessage		= 0x6EA42D80,
    opGetTicketsResponseMessage				= 0xBB567F98,
    opGetCommentsResponseMessage			= 0xEADB08CA,
    opGetArticleResponseMessage				= 0x934BAEE0,
    opCreateTicketResponseMessage			= 0x550A407A,
    opCancelTicketResponseMessage			= 0xD6FBF318,
    opAppendCommentResponseMessage			= 0xA04A3ECA
};

#endif
