/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UICALLBACK_H
#define ANH_ZONESERVER_UICALLBACK_H

#include "Utils/typedefs.h"
#include "UIEnums.h"

class UIWindow;

//======================================================================================================================

class UICallback
{
	public:
		virtual void	handleUIEvent(uint32 action,int32 element,string inputStr = "",UIWindow* window = NULL) {};
		virtual void	handleUIEvent(string leftValue = "", string rightValue = "", UIWindow* window = NULL){};
		virtual void	handleAsktoTeachSkill(uint32 action,int32 element,string inputStr,UIWindow* window, void* pupil, void* skill){};
		virtual void	handleSelectSkilltoTeach(uint32 action,int32 element,string inputStr,UIWindow* window, void* pupil, void* skillContainer){};
};

#endif 

