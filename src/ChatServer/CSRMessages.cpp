/*
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ChatMessageLib.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "Common/atMacroString.h"
#include "CSROpcodes.h"
#include <boost/lexical_cast.hpp>

void ChatMessageLib::sendConnectPlayerResponseMessage(DispatchClient* client)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opConnectPlayerResponseMessage);
	gMessageFactory->addUint32(0);
	Message* message = gMessageFactory->EndMessage();

	client->SendChannelA(message, client->getAccountId(), CR_Client, 2, false);
}

//======================================================================================================================

void ChatMessageLib::sendRequestCategoriesResponseMessage(DispatchClient *client, CategoryList* mCategoryList)
{
	std::vector<Category*>::iterator iter = mCategoryList->begin();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opRequestCategoriesResponseMessage);
	gMessageFactory->addUint32(0); //ErrorCode

	gMessageFactory->addUint32(mCategoryList->size()); //Category Count
	while (iter != mCategoryList->end())
	{
		gMessageFactory->addString((*iter)->mName); //Category Name
		gMessageFactory->addUint32((*iter)->mId); //Category Id

		SubCategoryList* subCats = (*iter)->GetSubCategories();
		std::vector<SubCategory*>::iterator subIter = subCats->begin();

		gMessageFactory->addUint32(subCats->size()); //SubCategory Count
		while (subIter != subCats->end())
		{
			gMessageFactory->addString((*subIter)->mName); //SubCategory Name
			gMessageFactory->addUint32((*subIter)->mId); //SubCategory Id
			gMessageFactory->addUint32(0); //Recursive list
			gMessageFactory->addUint8(1);
			gMessageFactory->addUint8(1);
			++subIter;
		}
		gMessageFactory->addUint8(1);
		gMessageFactory->addUint8(1);
		++iter;
	}
	Message* message = gMessageFactory->EndMessage();
	client->SendChannelA(message, client->getAccountId(), CR_Client, 3, false);
}

//======================================================================================================================

void ChatMessageLib::sendGetTicketsResponseMessage(DispatchClient* client, Ticket* ticket)
{

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opGetTicketsResponseMessage);

	gMessageFactory->addUint32(0); //ErrorCode
	gMessageFactory->addUint32(1); //Max tickets

	gMessageFactory->addUint32(1);//list size
	gMessageFactory->addUint32(ticket->mCategoryId);
	gMessageFactory->addUint32(ticket->mSubCategoryId);
	gMessageFactory->addString(ticket->mPlayer);
	gMessageFactory->addString(ticket->mComment);
	gMessageFactory->addString(ticket->mLanguage);
	gMessageFactory->addUint32(ticket->mId);
	gMessageFactory->addUint64(ticket->mLastModified);
	gMessageFactory->addUint8(ticket->mActivity);
	gMessageFactory->addUint8(ticket->mClosed);

	Message* message = gMessageFactory->EndMessage();
	delete(ticket); //Cleanup

	client->SendChannelA(message, client->getAccountId(), CR_Client, 4, false);
}

//======================================================================================================================

void ChatMessageLib::sendSearchKnowledgeBaseResponseMessage(DispatchClient* client, ArticleList* list)
{
	ArticleList::iterator iter = list->begin();
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSearchKnowledgeBaseResponseMessage);
	gMessageFactory	->addUint32(0); //ErrorCode

	gMessageFactory->addUint32(list->size()); //Article list size
	while (!list->empty())
	{
		gMessageFactory->addString((*iter)->mTitle);
		std::string idstring = boost::lexical_cast<std::string>((*iter)->mId); //Need to send the id as a string
		gMessageFactory->addString(BString((int8*)idstring.data()));
		gMessageFactory->addUint16(0); //Rank (doesn't matter);

		delete(*iter); //Delete as we go along
		list->erase(iter);
		iter = list->begin();
	}
	Message* message = gMessageFactory->EndMessage();
	delete(list); //Delete the list

	client->SendChannelA(message, client->getAccountId(), CR_Client, 3, false);
}

//======================================================================================================================

void ChatMessageLib::sendGetArticleResponseMessage(DispatchClient* client, Article* article)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opGetArticleResponseMessage);
	if (article == NULL)
	{
		gMessageFactory->addUint32(1); //Not found
		gMessageFactory->addUint32(0); //0 length string
	}
	else
	{
		gMessageFactory->addUint32(0); //Found
		gMessageFactory->addString(article->mBody);
	}
	Message* message = gMessageFactory->EndMessage();
	delete(article); //Cleanup

	client->SendChannelA(message, client->getAccountId(), CR_Client, 3, false);
}

//======================================================================================================================

void ChatMessageLib::sendGetCommentsResponseMessage(DispatchClient* client, CommentList* list)
{
	CommentList::iterator iter = list->begin();
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(0); //ErrorCode

	gMessageFactory->addUint32(list->size()); //Comment list size
	while (!list->empty())
	{
		gMessageFactory->addUint32((*iter)->mTicketId);
		gMessageFactory->addUint32((*iter)->mId);
		gMessageFactory->addUint8(1); //Read flag
		gMessageFactory->addString((*iter)->mText);
		gMessageFactory->addString((*iter)->mAuthor);

		delete(*iter); //Delete as we go along
		list->erase(iter);
		iter = list->begin();
	}
	Message* message = gMessageFactory->EndMessage();
	delete(list); //Then delete the lsit

	client->SendChannelA(message, client->getAccountId(), CR_Client	, 3, false);
}