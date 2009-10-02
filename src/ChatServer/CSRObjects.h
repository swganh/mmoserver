/*
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CSROBJECT_H
#define ANH_ZONESERVER_CSROBJECT_H

class Category;
class SubCategory;
class Comment;

typedef std::vector<SubCategory*> SubCategoryList;

//======================================================================================================================

class Category
{
public:

	Category(){}
	~Category(){}

	SubCategoryList* GetSubCategories() { return &mSubCategories; }

	uint32 mId;
	string mName;

private:
	SubCategoryList mSubCategories;

};

//======================================================================================================================

class SubCategory
{
public:

	SubCategory(){}
	~SubCategory(){}

	uint32			mId;
	string			mName;
};

//======================================================================================================================

class Article
{
public:

	Article(){}
	~Article(){}

	uint32			mId;
	string			mTitle;
	string			mBody;
};

//======================================================================================================================

class Ticket
{
public:

	Ticket(){}
	~Ticket(){}

	uint32		mId;
	string		mPlayer;
	uint32		mCategoryId;
	uint32		mSubCategoryId;
	string		mComment;
	string		mInfo;
	string		mHarrasingUser;
	string		mLanguage;
	uint8		mBugReport;
	uint8		mClosed;
	uint8		mActivity;
	uint64		mLastModified;
	CommentList mCommentList;
};

//======================================================================================================================

class Comment
{
public:
	Comment(){}
	~Comment(){}

	string	mText;
	uint32	mId;
	uint32	mTicketId;
	string  mAuthor;
};

#endif