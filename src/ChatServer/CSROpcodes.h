/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

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
	opAppendCommentResponseMessage			= 0xA04A3ECA,
};

#endif