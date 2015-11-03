/*
** Lua binding: LuaInterface
** Generated automatically by tolua++-1.0.92 on 08/28/09 02:28:22.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_LuaInterface_open (lua_State* tolua_S);

#include "DatabaseManager/DatabaseCallback.h"
#include "ZoneServer/ObjectFactoryCallback.h"
#include "Utils/TimerCallback.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/Object.h"
#include "ScriptEngine/Script.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/ObjectControllerCommandMap.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
    tolua_usertype(tolua_S,"Tutorial");
    tolua_usertype(tolua_S,"ScriptSupport");
    tolua_usertype(tolua_S,"WorldManager");
    tolua_usertype(tolua_S,"Object");
    tolua_usertype(tolua_S,"ScriptEngine");
    tolua_usertype(tolua_S,"NPCObject");
    tolua_usertype(tolua_S,"ObjectControllerCommandMap");
    tolua_usertype(tolua_S,"Script");
}

/* method: getSingletonPtr of class  WorldManager */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_WorldManager_getSingletonPtr00
static int tolua_LuaInterface_WorldManager_getSingletonPtr00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"WorldManager",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        {
            WorldManager* tolua_ret = (WorldManager*)  WorldManager::getSingletonPtr();
            tolua_pushusertype(tolua_S,(void*)tolua_ret,"WorldManager");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getSingletonPtr'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getZoneId of class  WorldManager */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_WorldManager_getZoneId00
static int tolua_LuaInterface_WorldManager_getZoneId00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"WorldManager",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        WorldManager* self = (WorldManager*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getZoneId'",NULL);
#endif
        {
            unsigned long tolua_ret = ( unsigned long)  self->getZoneId();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getZoneId'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: updateWeather of class  WorldManager */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_WorldManager_updateWeather00
static int tolua_LuaInterface_WorldManager_updateWeather00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"WorldManager",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,6,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        WorldManager* self = (WorldManager*)  tolua_tousertype(tolua_S,1,0);
        float cloudX = ((float)  tolua_tonumber(tolua_S,2,0));
        float cloudY = ((float)  tolua_tonumber(tolua_S,3,0));
        float cloudZ = ((float)  tolua_tonumber(tolua_S,4,0));
        unsigned long weatherType = (( unsigned long)  tolua_tonumber(tolua_S,5,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'updateWeather'",NULL);
#endif
        {
            self->updateWeather(cloudX,cloudY,cloudZ,weatherType);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'updateWeather'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: zoneSystemMessage of class  WorldManager */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_WorldManager_zoneSystemMessage00
static int tolua_LuaInterface_WorldManager_zoneSystemMessage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"WorldManager",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        WorldManager* self = (WorldManager*)  tolua_tousertype(tolua_S,1,0);
        std::string message = ((std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'zoneSystemMessage'",NULL);
#endif
        {
            self->zoneSystemMessage(message);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'zoneSystemMessage'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ScriptRegisterEvent of class  WorldManager */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_WorldManager_ScriptRegisterEvent00
static int tolua_LuaInterface_WorldManager_ScriptRegisterEvent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"WorldManager",0,&tolua_err) ||
        !tolua_isuserdata(tolua_S,2,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        WorldManager* self = (WorldManager*)  tolua_tousertype(tolua_S,1,0);
        void* script = ((void*)  tolua_touserdata(tolua_S,2,0));
        std::string eventFunction = ((std::string)  tolua_tocppstring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ScriptRegisterEvent'",NULL);
#endif
        {
            self->ScriptRegisterEvent(script,eventFunction);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'ScriptRegisterEvent'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getObjectById of class  WorldManager */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_WorldManager_getObjectById00
static int tolua_LuaInterface_WorldManager_getObjectById00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"WorldManager",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        WorldManager* self = (WorldManager*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long objId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getObjectById'",NULL);
#endif
        {
            Object* tolua_ret = (Object*)  self->getObjectById(objId);
            tolua_pushusertype(tolua_S,(void*)tolua_ret,"Object");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getObjectById'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getId of class  Object */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Object_getId00
static int tolua_LuaInterface_Object_getId00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Object",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Object* self = (Object*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getId'",NULL);
#endif
        {
            unsigned long long tolua_ret = ( unsigned long long)  self->getId();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getId'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getParentId of class  Object */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Object_getParentId00
static int tolua_LuaInterface_Object_getParentId00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Object",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Object* self = (Object*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getParentId'",NULL);
#endif
        {
            unsigned long long tolua_ret = ( unsigned long long)  self->getParentId();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getParentId'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSubZoneId of class  Object */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Object_getSubZoneId00
static int tolua_LuaInterface_Object_getSubZoneId00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Object",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Object* self = (Object*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getSubZoneId'",NULL);
#endif
        {
            unsigned long tolua_ret = ( unsigned long)  self->getSubZoneId();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getSubZoneId'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ScriptRegisterEvent of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_ScriptRegisterEvent00
static int tolua_LuaInterface_Tutorial_ScriptRegisterEvent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isuserdata(tolua_S,2,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        void* script = ((void*)  tolua_touserdata(tolua_S,2,0));
        std::string eventFunction = ((std::string)  tolua_tocppstring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ScriptRegisterEvent'",NULL);
#endif
        {
            self->ScriptRegisterEvent(script,eventFunction);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'ScriptRegisterEvent'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: scriptPlayMusic of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_scriptPlayMusic00
static int tolua_LuaInterface_Tutorial_scriptPlayMusic00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long soundId = (( unsigned long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'scriptPlayMusic'",NULL);
#endif
        {
            self->scriptPlayMusic(soundId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'scriptPlayMusic'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: scriptSystemMessage of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_scriptSystemMessage00
static int tolua_LuaInterface_Tutorial_scriptSystemMessage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        std::string message = ((std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'scriptSystemMessage'",NULL);
#endif
        {
            self->scriptSystemMessage(message);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'scriptSystemMessage'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: updateTutorial of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_updateTutorial00
static int tolua_LuaInterface_Tutorial_updateTutorial00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        std::string customMessage = ((std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'updateTutorial'",NULL);
#endif
        {
            self->updateTutorial(customMessage);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'updateTutorial'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: openHolocron of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_openHolocron00
static int tolua_LuaInterface_Tutorial_openHolocron00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'openHolocron'",NULL);
#endif
        {
            self->openHolocron();
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'openHolocron'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: spatialChat of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_spatialChat00
static int tolua_LuaInterface_Tutorial_spatialChat00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long targetId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
        std::string chatMsg = ((std::string)  tolua_tocppstring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'spatialChat'",NULL);
#endif
        {
            self->spatialChat(targetId,chatMsg);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'spatialChat'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: spatialChatShout of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_spatialChatShout00
static int tolua_LuaInterface_Tutorial_spatialChatShout00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long targetId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
        std::string chatMsg = ((std::string)  tolua_tocppstring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'spatialChatShout'",NULL);
#endif
        {
            self->spatialChatShout(targetId,chatMsg);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'spatialChatShout'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: enableHudElement of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_enableHudElement00
static int tolua_LuaInterface_Tutorial_enableHudElement00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        std::string customMessage = ((std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'enableHudElement'",NULL);
#endif
        {
            self->enableHudElement(customMessage);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'enableHudElement'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: disableHudElement of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_disableHudElement00
static int tolua_LuaInterface_Tutorial_disableHudElement00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        std::string customMessage = ((std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'disableHudElement'",NULL);
#endif
        {
            self->disableHudElement(customMessage);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'disableHudElement'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isZoomCamera of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isZoomCamera00
static int tolua_LuaInterface_Tutorial_isZoomCamera00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isZoomCamera'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isZoomCamera();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isZoomCamera'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isChatBox of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isChatBox00
static int tolua_LuaInterface_Tutorial_isChatBox00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isChatBox'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isChatBox();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isChatBox'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isFoodSelected of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isFoodSelected00
static int tolua_LuaInterface_Tutorial_isFoodSelected00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isFoodSelected'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isFoodSelected();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isFoodSelected'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isFoodUsed of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isFoodUsed00
static int tolua_LuaInterface_Tutorial_isFoodUsed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isFoodUsed'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isFoodUsed();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isFoodUsed'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isCloneDataSaved of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isCloneDataSaved00
static int tolua_LuaInterface_Tutorial_isCloneDataSaved00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isCloneDataSaved'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isCloneDataSaved();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isCloneDataSaved'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isItemsInsured of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isItemsInsured00
static int tolua_LuaInterface_Tutorial_isItemsInsured00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isItemsInsured'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isItemsInsured();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isItemsInsured'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isCloseHolocron of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isCloseHolocron00
static int tolua_LuaInterface_Tutorial_isCloseHolocron00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isCloseHolocron'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isCloseHolocron();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isCloseHolocron'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isChangeLookAtTarget of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isChangeLookAtTarget00
static int tolua_LuaInterface_Tutorial_isChangeLookAtTarget00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isChangeLookAtTarget'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isChangeLookAtTarget();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isChangeLookAtTarget'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isOpenInventory of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isOpenInventory00
static int tolua_LuaInterface_Tutorial_isOpenInventory00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isOpenInventory'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isOpenInventory();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isOpenInventory'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isCloseInventory of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isCloseInventory00
static int tolua_LuaInterface_Tutorial_isCloseInventory00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isCloseInventory'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isCloseInventory();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isCloseInventory'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getState of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getState00
static int tolua_LuaInterface_Tutorial_getState00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getState'",NULL);
#endif
        {
            unsigned long tolua_ret = ( unsigned long)  self->getState();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getState'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setState of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_setState00
static int tolua_LuaInterface_Tutorial_setState00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long state = (( unsigned long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setState'",NULL);
#endif
        {
            self->setState(state);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'setState'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSubState of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getSubState00
static int tolua_LuaInterface_Tutorial_getSubState00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getSubState'",NULL);
#endif
        {
            unsigned long tolua_ret = ( unsigned long)  self->getSubState();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getSubState'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setSubState of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_setSubState00
static int tolua_LuaInterface_Tutorial_setSubState00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long subState = (( unsigned long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSubState'",NULL);
#endif
        {
            self->setSubState(subState);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'setSubState'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: enableTutorial of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_enableTutorial00
static int tolua_LuaInterface_Tutorial_enableTutorial00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'enableTutorial'",NULL);
#endif
        {
            self->enableTutorial();
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'enableTutorial'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: disableTutorial of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_disableTutorial00
static int tolua_LuaInterface_Tutorial_disableTutorial00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'disableTutorial'",NULL);
#endif
        {
            self->disableTutorial();
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'disableTutorial'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPlayerPosX of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getPlayerPosX00
static int tolua_LuaInterface_Tutorial_getPlayerPosX00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPlayerPosX'",NULL);
#endif
        {
            float tolua_ret = (float)  self->getPlayerPosX();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getPlayerPosX'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPlayerPosY of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getPlayerPosY00
static int tolua_LuaInterface_Tutorial_getPlayerPosY00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPlayerPosY'",NULL);
#endif
        {
            float tolua_ret = (float)  self->getPlayerPosY();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getPlayerPosY'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPlayerPosZ of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getPlayerPosZ00
static int tolua_LuaInterface_Tutorial_getPlayerPosZ00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPlayerPosZ'",NULL);
#endif
        {
            float tolua_ret = (float)  self->getPlayerPosZ();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getPlayerPosZ'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPlayerPosToObject of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getPlayerPosToObject00
static int tolua_LuaInterface_Tutorial_getPlayerPosToObject00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long objectId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPlayerPosToObject'",NULL);
#endif
        {
            float tolua_ret = (float)  self->getPlayerPosToObject(objectId);
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getPlayerPosToObject'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: enableNpcConversationEvent of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_enableNpcConversationEvent00
static int tolua_LuaInterface_Tutorial_enableNpcConversationEvent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long objectId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'enableNpcConversationEvent'",NULL);
#endif
        {
            self->enableNpcConversationEvent(objectId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'enableNpcConversationEvent'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isNpcConversationStarted of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isNpcConversationStarted00
static int tolua_LuaInterface_Tutorial_isNpcConversationStarted00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isNpcConversationStarted'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isNpcConversationStarted(npcId);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isNpcConversationStarted'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isNpcConversationEnded of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isNpcConversationEnded00
static int tolua_LuaInterface_Tutorial_isNpcConversationEnded00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isNpcConversationEnded'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isNpcConversationEnded(npcId);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isNpcConversationEnded'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: enableItemContainerEvent of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_enableItemContainerEvent00
static int tolua_LuaInterface_Tutorial_enableItemContainerEvent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long objectId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'enableItemContainerEvent'",NULL);
#endif
        {
            self->enableItemContainerEvent(objectId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'enableItemContainerEvent'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isContainerOpen of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isContainerOpen00
static int tolua_LuaInterface_Tutorial_isContainerOpen00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long containerId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isContainerOpen'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isContainerOpen(containerId);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isContainerOpen'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isContainerClosed of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isContainerClosed00
static int tolua_LuaInterface_Tutorial_isContainerClosed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long containerId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isContainerClosed'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isContainerClosed(containerId);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isContainerClosed'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isContainerEmpty of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isContainerEmpty00
static int tolua_LuaInterface_Tutorial_isContainerEmpty00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long containerId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isContainerEmpty'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isContainerEmpty(containerId);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isContainerEmpty'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isItemTransferedFromContainer of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isItemTransferedFromContainer00
static int tolua_LuaInterface_Tutorial_isItemTransferedFromContainer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long containerId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isItemTransferedFromContainer'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isItemTransferedFromContainer(containerId);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isItemTransferedFromContainer'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPlayer of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getPlayer00
static int tolua_LuaInterface_Tutorial_getPlayer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPlayer'",NULL);
#endif
        {
            unsigned long long tolua_ret = ( unsigned long long)  self->getPlayer();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getPlayer'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: addQuestWeapon of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_addQuestWeapon00
static int tolua_LuaInterface_Tutorial_addQuestWeapon00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long familyId = (( unsigned long)  tolua_tonumber(tolua_S,2,0));
        unsigned long typeId = (( unsigned long)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'addQuestWeapon'",NULL);
#endif
        {
            self->addQuestWeapon(familyId,typeId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'addQuestWeapon'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcStopFight of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_npcStopFight00
static int tolua_LuaInterface_Tutorial_npcStopFight00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcStopFight'",NULL);
#endif
        {
            self->npcStopFight(npcId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcStopFight'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isLowHam of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isLowHam00
static int tolua_LuaInterface_Tutorial_isLowHam00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
        long hamLevel = ((  long)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isLowHam'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isLowHam(npcId,hamLevel);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isLowHam'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSkillTrainerTypeId of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getSkillTrainerTypeId00
static int tolua_LuaInterface_Tutorial_getSkillTrainerTypeId00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getSkillTrainerTypeId'",NULL);
#endif
        {
            unsigned long long tolua_ret = ( unsigned long long)  self->getSkillTrainerTypeId();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getSkillTrainerTypeId'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isPlayerTrained of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_isPlayerTrained00
static int tolua_LuaInterface_Tutorial_isPlayerTrained00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isPlayerTrained'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isPlayerTrained();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isPlayerTrained'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTutorialRefugeeTaunts of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_setTutorialRefugeeTaunts00
static int tolua_LuaInterface_Tutorial_setTutorialRefugeeTaunts00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setTutorialRefugeeTaunts'",NULL);
#endif
        {
            self->setTutorialRefugeeTaunts(npcId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'setTutorialRefugeeTaunts'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTutorialCelebTaunts of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_setTutorialCelebTaunts00
static int tolua_LuaInterface_Tutorial_setTutorialCelebTaunts00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setTutorialCelebTaunts'",NULL);
#endif
        {
            self->setTutorialCelebTaunts(npcId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'setTutorialCelebTaunts'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: makeCreatureAttackable of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_makeCreatureAttackable00
static int tolua_LuaInterface_Tutorial_makeCreatureAttackable00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'makeCreatureAttackable'",NULL);
#endif
        {
            self->makeCreatureAttackable(npcId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'makeCreatureAttackable'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcSendAnimation of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_npcSendAnimation00
static int tolua_LuaInterface_Tutorial_npcSendAnimation00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
        unsigned long animId = (( unsigned long)  tolua_tonumber(tolua_S,3,0));
        unsigned long long targetId = (( unsigned long long)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcSendAnimation'",NULL);
#endif
        {
            self->npcSendAnimation(npcId,animId,targetId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcSendAnimation'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getReady of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getReady00
static int tolua_LuaInterface_Tutorial_getReady00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getReady'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->getReady();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getReady'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getRoom of class  Tutorial */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_Tutorial_getRoom00
static int tolua_LuaInterface_Tutorial_getRoom00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"Tutorial",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        Tutorial* self = (Tutorial*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getRoom'",NULL);
#endif
        {
            unsigned long tolua_ret = ( unsigned long)  self->getRoom();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getRoom'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSingletonPtr of class  ObjectControllerCommandMap */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ObjectControllerCommandMap_getSingletonPtr00
static int tolua_LuaInterface_ObjectControllerCommandMap_getSingletonPtr00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"ObjectControllerCommandMap",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        {
            ObjectControllerCommandMap* tolua_ret = (ObjectControllerCommandMap*)  ObjectControllerCommandMap::getSingletonPtr();
            tolua_pushusertype(tolua_S,(void*)tolua_ret,"ObjectControllerCommandMap");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getSingletonPtr'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: ScriptRegisterEvent of class  ObjectControllerCommandMap */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ObjectControllerCommandMap_ScriptRegisterEvent00
static int tolua_LuaInterface_ObjectControllerCommandMap_ScriptRegisterEvent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ObjectControllerCommandMap",0,&tolua_err) ||
        !tolua_isuserdata(tolua_S,2,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ObjectControllerCommandMap* self = (ObjectControllerCommandMap*)  tolua_tousertype(tolua_S,1,0);
        void* script = ((void*)  tolua_touserdata(tolua_S,2,0));
        std::string eventFunction = ((std::string)  tolua_tocppstring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ScriptRegisterEvent'",NULL);
#endif
        {
            self->ScriptRegisterEvent(script,eventFunction);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'ScriptRegisterEvent'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Init of class  ScriptEngine */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptEngine_Init00
static int tolua_LuaInterface_ScriptEngine_Init00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"ScriptEngine",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        {
            ScriptEngine* tolua_ret = (ScriptEngine*)  ScriptEngine::Init();
            tolua_pushusertype(tolua_S,(void*)tolua_ret,"ScriptEngine");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'Init'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getTutorial of class  ScriptEngine */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptEngine_getTutorial00
static int tolua_LuaInterface_ScriptEngine_getTutorial00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptEngine",0,&tolua_err) ||
        !tolua_isuserdata(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptEngine* self = (ScriptEngine*)  tolua_tousertype(tolua_S,1,0);
        void* script = ((void*)  tolua_touserdata(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getTutorial'",NULL);
#endif
        {
            Tutorial* tolua_ret = (Tutorial*)  self->getTutorial(script);
            tolua_pushusertype(tolua_S,(void*)tolua_ret,"Tutorial");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getTutorial'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Instance of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_Instance00
static int tolua_LuaInterface_ScriptSupport_Instance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        {
            ScriptSupport* tolua_ret = (ScriptSupport*)  ScriptSupport::Instance();
            tolua_pushusertype(tolua_S,(void*)tolua_ret,"ScriptSupport");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'Instance'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcCreate of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcCreate00
static int tolua_LuaInterface_ScriptSupport_npcCreate00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcCreate'",NULL);
#endif
        {
            unsigned long long tolua_ret = ( unsigned long long)  self->npcCreate(npcId);
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcCreate'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcGetObject of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcGetObject00
static int tolua_LuaInterface_ScriptSupport_npcGetObject00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long id = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcGetObject'",NULL);
#endif
        {
            NPCObject* tolua_ret = (NPCObject*)  self->npcGetObject(id);
            tolua_pushusertype(tolua_S,(void*)tolua_ret,"NPCObject");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcGetObject'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcSpawn of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcSpawn00
static int tolua_LuaInterface_ScriptSupport_npcSpawn00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,5,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,6,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,8,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,9,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,10,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,11,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,12,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,3,0));
        unsigned long long cellForSpawn = (( unsigned long long)  tolua_tonumber(tolua_S,4,0));
        std::string firstname = ((std::string)  tolua_tocppstring(tolua_S,5,0));
        std::string lastname = ((std::string)  tolua_tocppstring(tolua_S,6,0));
        float dirY = ((float)  tolua_tonumber(tolua_S,7,0));
        float dirW = ((float)  tolua_tonumber(tolua_S,8,0));
        float posX = ((float)  tolua_tonumber(tolua_S,9,0));
        float posY = ((float)  tolua_tonumber(tolua_S,10,0));
        float posZ = ((float)  tolua_tonumber(tolua_S,11,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcSpawn'",NULL);
#endif
        {
            self->npcSpawn(npc,npcId,cellForSpawn,firstname,lastname,dirY,dirW,posX,posY,posZ);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcSpawn'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcSpawnPrivate of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcSpawnPrivate00
static int tolua_LuaInterface_ScriptSupport_npcSpawnPrivate00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,6,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,7,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,8,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,9,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,10,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,11,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,12,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,13,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,3,0));
        unsigned long long npcPrivateOwnerId = (( unsigned long long)  tolua_tonumber(tolua_S,4,0));
        unsigned long long cellForSpawn = (( unsigned long long)  tolua_tonumber(tolua_S,5,0));
        std::string firstname = ((std::string)  tolua_tocppstring(tolua_S,6,0));
        std::string lastname = ((std::string)  tolua_tocppstring(tolua_S,7,0));
        float dirY = ((float)  tolua_tonumber(tolua_S,8,0));
        float dirW = ((float)  tolua_tonumber(tolua_S,9,0));
        float posX = ((float)  tolua_tonumber(tolua_S,10,0));
        float posY = ((float)  tolua_tonumber(tolua_S,11,0));
        float posZ = ((float)  tolua_tonumber(tolua_S,12,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcSpawnPrivate'",NULL);
#endif
        {
            self->npcSpawnPrivate(npc,npcId,npcPrivateOwnerId,cellForSpawn,firstname,lastname,dirY,dirW,posX,posY,posZ);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcSpawnPrivate'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcSpawnPersistent of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcSpawnPersistent00
static int tolua_LuaInterface_ScriptSupport_npcSpawnPersistent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,5,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,6,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,8,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,9,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,10,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,11,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,12,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,13,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,14,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,3,0));
        unsigned long long cellForSpawn = (( unsigned long long)  tolua_tonumber(tolua_S,4,0));
        std::string firstname = ((std::string)  tolua_tocppstring(tolua_S,5,0));
        std::string lastname = ((std::string)  tolua_tocppstring(tolua_S,6,0));
        float dirY = ((float)  tolua_tonumber(tolua_S,7,0));
        float dirW = ((float)  tolua_tonumber(tolua_S,8,0));
        float posX = ((float)  tolua_tonumber(tolua_S,9,0));
        float posY = ((float)  tolua_tonumber(tolua_S,10,0));
        float posZ = ((float)  tolua_tonumber(tolua_S,11,0));
        unsigned long long respawnPeriod = (( unsigned long long)  tolua_tonumber(tolua_S,12,0));
        unsigned long long templateId = (( unsigned long long)  tolua_tonumber(tolua_S,13,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcSpawnPersistent'",NULL);
#endif
        {
            self->npcSpawnPersistent(npc,npcId,cellForSpawn,firstname,lastname,dirY,dirW,posX,posY,posZ,respawnPeriod,templateId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcSpawnPersistent'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: lairSpawn of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_lairSpawn00
static int tolua_LuaInterface_ScriptSupport_lairSpawn00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long lairTypeId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'lairSpawn'",NULL);
#endif
        {
            self->lairSpawn(lairTypeId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'lairSpawn'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcDirection of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcDirection00
static int tolua_LuaInterface_ScriptSupport_npcDirection00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        float deltaX = ((float)  tolua_tonumber(tolua_S,3,0));
        float deltaZ = ((float)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcDirection'",NULL);
#endif
        {
            self->npcDirection(npc,deltaX,deltaZ);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcDirection'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcMove of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcMove00
static int tolua_LuaInterface_ScriptSupport_npcMove00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,6,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        float posX = ((float)  tolua_tonumber(tolua_S,3,0));
        float posY = ((float)  tolua_tonumber(tolua_S,4,0));
        float posZ = ((float)  tolua_tonumber(tolua_S,5,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcMove'",NULL);
#endif
        {
            self->npcMove(npc,posX,posY,posZ);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcMove'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcMoveToZone of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcMoveToZone00
static int tolua_LuaInterface_ScriptSupport_npcMoveToZone00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,7,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        unsigned long long zoneId = (( unsigned long long)  tolua_tonumber(tolua_S,3,0));
        float posX = ((float)  tolua_tonumber(tolua_S,4,0));
        float posY = ((float)  tolua_tonumber(tolua_S,5,0));
        float posZ = ((float)  tolua_tonumber(tolua_S,6,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcMoveToZone'",NULL);
#endif
        {
            self->npcMoveToZone(npc,zoneId,posX,posY,posZ);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcMoveToZone'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcFormationPosition of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcFormationPosition00
static int tolua_LuaInterface_ScriptSupport_npcFormationPosition00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npcMember = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        float xOffset = ((float)  tolua_tonumber(tolua_S,3,0));
        float zOffset = ((float)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcFormationPosition'",NULL);
#endif
        {
            self->npcFormationPosition(npcMember,xOffset,zOffset);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcFormationPosition'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcClonePosition of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcClonePosition00
static int tolua_LuaInterface_ScriptSupport_npcClonePosition00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,3,"NPCObject",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npcDest = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        NPCObject* npcSrc = ((NPCObject*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcClonePosition'",NULL);
#endif
        {
            self->npcClonePosition(npcDest,npcSrc);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcClonePosition'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcFormationMove of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcFormationMove00
static int tolua_LuaInterface_ScriptSupport_npcFormationMove00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,6,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        float posX = ((float)  tolua_tonumber(tolua_S,3,0));
        float posY = ((float)  tolua_tonumber(tolua_S,4,0));
        float posZ = ((float)  tolua_tonumber(tolua_S,5,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcFormationMove'",NULL);
#endif
        {
            self->npcFormationMove(npc,posX,posY,posZ);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcFormationMove'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcFormationMoveEx of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcFormationMoveEx00
static int tolua_LuaInterface_ScriptSupport_npcFormationMoveEx00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,8,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        float posX = ((float)  tolua_tonumber(tolua_S,3,0));
        float posY = ((float)  tolua_tonumber(tolua_S,4,0));
        float posZ = ((float)  tolua_tonumber(tolua_S,5,0));
        float xOffset = ((float)  tolua_tonumber(tolua_S,6,0));
        float zOffset = ((float)  tolua_tonumber(tolua_S,7,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcFormationMoveEx'",NULL);
#endif
        {
            self->npcFormationMoveEx(npc,posX,posY,posZ,xOffset,zOffset);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcFormationMoveEx'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: itemCreate of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_itemCreate00
static int tolua_LuaInterface_ScriptSupport_itemCreate00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long itemTypesId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'itemCreate'",NULL);
#endif
        {
            unsigned long long tolua_ret = ( unsigned long long)  self->itemCreate(itemTypesId);
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'itemCreate'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: itemPopulateInventory of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_itemPopulateInventory00
static int tolua_LuaInterface_ScriptSupport_itemPopulateInventory00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long itemId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,3,0));
        unsigned long long playerId = (( unsigned long long)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'itemPopulateInventory'",NULL);
#endif
        {
            self->itemPopulateInventory(itemId,npcId,playerId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'itemPopulateInventory'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: objectIsReady of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_objectIsReady00
static int tolua_LuaInterface_ScriptSupport_objectIsReady00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'objectIsReady'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->objectIsReady(npcId);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'objectIsReady'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getObject of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_getObject00
static int tolua_LuaInterface_ScriptSupport_getObject00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long id = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getObject'",NULL);
#endif
        {
            Object* tolua_ret = (Object*)  self->getObject(id);
            tolua_pushusertype(tolua_S,(void*)tolua_ret,"Object");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getObject'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: eraseObject of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_eraseObject00
static int tolua_LuaInterface_ScriptSupport_eraseObject00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long id = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'eraseObject'",NULL);
#endif
        {
            self->eraseObject(id);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'eraseObject'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getZoneId of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_getZoneId00
static int tolua_LuaInterface_ScriptSupport_getZoneId00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getZoneId'",NULL);
#endif
        {
            unsigned long tolua_ret = ( unsigned long)  self->getZoneId();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getZoneId'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: enableInstance of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_enableInstance00
static int tolua_LuaInterface_ScriptSupport_enableInstance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'enableInstance'",NULL);
#endif
        {
            self->enableInstance();
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'enableInstance'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isInstance of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_isInstance00
static int tolua_LuaInterface_ScriptSupport_isInstance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isInstance'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->isInstance();
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'isInstance'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: scriptPlayMusic of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_scriptPlayMusic00
static int tolua_LuaInterface_ScriptSupport_scriptPlayMusic00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isusertype(tolua_S,3,"NPCObject",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long soundId = (( unsigned long)  tolua_tonumber(tolua_S,2,0));
        NPCObject* creatureObject = ((NPCObject*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'scriptPlayMusic'",NULL);
#endif
        {
            self->scriptPlayMusic(soundId,creatureObject);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'scriptPlayMusic'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getTarget of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_getTarget00
static int tolua_LuaInterface_ScriptSupport_getTarget00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long playerId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getTarget'",NULL);
#endif
        {
            unsigned long long tolua_ret = ( unsigned long long)  self->getTarget(playerId);
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getTarget'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getParentOfTarget of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_getParentOfTarget00
static int tolua_LuaInterface_ScriptSupport_getParentOfTarget00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long playerId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getParentOfTarget'",NULL);
#endif
        {
            unsigned long long tolua_ret = ( unsigned long long)  self->getParentOfTarget(playerId);
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getParentOfTarget'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: sendFlyText of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_sendFlyText00
static int tolua_LuaInterface_ScriptSupport_sendFlyText00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,4,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,5,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,7,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,8,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,9,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,10,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long targetId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
        unsigned long long playerId = (( unsigned long long)  tolua_tonumber(tolua_S,3,0));
        std::string stfFile = ((std::string)  tolua_tocppstring(tolua_S,4,0));
        std::string stfVar = ((std::string)  tolua_tocppstring(tolua_S,5,0));
        unsigned char red = (( unsigned char)  tolua_tonumber(tolua_S,6,0));
        unsigned char green = (( unsigned char)  tolua_tonumber(tolua_S,7,0));
        unsigned char blue = (( unsigned char)  tolua_tonumber(tolua_S,8,0));
        unsigned char duration = (( unsigned char)  tolua_tonumber(tolua_S,9,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'sendFlyText'",NULL);
#endif
        {
            self->sendFlyText(targetId,playerId,stfFile,stfVar,red,green,blue,duration);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'sendFlyText'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: scriptSystemMessage of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_scriptSystemMessage00
static int tolua_LuaInterface_ScriptSupport_scriptSystemMessage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long playerId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
        unsigned long long targetId = (( unsigned long long)  tolua_tonumber(tolua_S,3,0));
        std::string message = ((std::string)  tolua_tocppstring(tolua_S,4,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'scriptSystemMessage'",NULL);
#endif
        {
            self->scriptSystemMessage(playerId,targetId,message);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'scriptSystemMessage'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setPlayerPosition of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_setPlayerPosition00
static int tolua_LuaInterface_ScriptSupport_setPlayerPosition00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,7,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long playerId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
        unsigned long long cellId = (( unsigned long long)  tolua_tonumber(tolua_S,3,0));
        float posX = ((float)  tolua_tonumber(tolua_S,4,0));
        float posY = ((float)  tolua_tonumber(tolua_S,5,0));
        float posZ = ((float)  tolua_tonumber(tolua_S,6,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPlayerPosition'",NULL);
#endif
        {
            self->setPlayerPosition(playerId,cellId,posX,posY,posZ);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'setPlayerPosition'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcInCombat of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcInCombat00
static int tolua_LuaInterface_ScriptSupport_npcInCombat00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcInCombat'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->npcInCombat(npcId);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcInCombat'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcIsDead of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcIsDead00
static int tolua_LuaInterface_ScriptSupport_npcIsDead00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcIsDead'",NULL);
#endif
        {
            bool tolua_ret = (bool)  self->npcIsDead(npcId);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcIsDead'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcKill of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcKill00
static int tolua_LuaInterface_ScriptSupport_npcKill00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        unsigned long long npcId = (( unsigned long long)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcKill'",NULL);
#endif
        {
            self->npcKill(npcId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcKill'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: npcTestDir of class  ScriptSupport */
#ifndef TOLUA_DISABLE_tolua_LuaInterface_ScriptSupport_npcTestDir00
static int tolua_LuaInterface_ScriptSupport_npcTestDir00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"ScriptSupport",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"NPCObject",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
    )
        goto tolua_lerror;
    else
#endif
    {
        ScriptSupport* self = (ScriptSupport*)  tolua_tousertype(tolua_S,1,0);
        NPCObject* npc = ((NPCObject*)  tolua_tousertype(tolua_S,2,0));
        float dirX = ((float)  tolua_tonumber(tolua_S,3,0));
        float dirZ = ((float)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'npcTestDir'",NULL);
#endif
        {
            self->npcTestDir(npc,dirX,dirZ);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'npcTestDir'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_LuaInterface_open (lua_State* tolua_S)
{
    tolua_open(tolua_S);
    tolua_reg_types(tolua_S);
    tolua_module(tolua_S,NULL,0);
    tolua_beginmodule(tolua_S,NULL);
    tolua_cclass(tolua_S,"WorldManager","WorldManager","",NULL);
    tolua_beginmodule(tolua_S,"WorldManager");
    tolua_function(tolua_S,"getSingletonPtr",tolua_LuaInterface_WorldManager_getSingletonPtr00);
    tolua_function(tolua_S,"getZoneId",tolua_LuaInterface_WorldManager_getZoneId00);
    tolua_function(tolua_S,"updateWeather",tolua_LuaInterface_WorldManager_updateWeather00);
    tolua_function(tolua_S,"zoneSystemMessage",tolua_LuaInterface_WorldManager_zoneSystemMessage00);
    tolua_function(tolua_S,"ScriptRegisterEvent",tolua_LuaInterface_WorldManager_ScriptRegisterEvent00);
    tolua_function(tolua_S,"getObjectById",tolua_LuaInterface_WorldManager_getObjectById00);
    tolua_endmodule(tolua_S);
    tolua_cclass(tolua_S,"Script","Script","",NULL);
    tolua_beginmodule(tolua_S,"Script");
    tolua_endmodule(tolua_S);
    tolua_cclass(tolua_S,"Object","Object","",NULL);
    tolua_beginmodule(tolua_S,"Object");
    tolua_function(tolua_S,"getId",tolua_LuaInterface_Object_getId00);
    tolua_function(tolua_S,"getParentId",tolua_LuaInterface_Object_getParentId00);
    tolua_function(tolua_S,"getSubZoneId",tolua_LuaInterface_Object_getSubZoneId00);
    tolua_endmodule(tolua_S);
    tolua_cclass(tolua_S,"Tutorial","Tutorial","",NULL);
    tolua_beginmodule(tolua_S,"Tutorial");
    tolua_function(tolua_S,"ScriptRegisterEvent",tolua_LuaInterface_Tutorial_ScriptRegisterEvent00);
    tolua_function(tolua_S,"scriptPlayMusic",tolua_LuaInterface_Tutorial_scriptPlayMusic00);
    tolua_function(tolua_S,"scriptSystemMessage",tolua_LuaInterface_Tutorial_scriptSystemMessage00);
    tolua_function(tolua_S,"updateTutorial",tolua_LuaInterface_Tutorial_updateTutorial00);
    tolua_function(tolua_S,"openHolocron",tolua_LuaInterface_Tutorial_openHolocron00);
    tolua_function(tolua_S,"spatialChat",tolua_LuaInterface_Tutorial_spatialChat00);
    tolua_function(tolua_S,"spatialChatShout",tolua_LuaInterface_Tutorial_spatialChatShout00);
    tolua_function(tolua_S,"enableHudElement",tolua_LuaInterface_Tutorial_enableHudElement00);
    tolua_function(tolua_S,"disableHudElement",tolua_LuaInterface_Tutorial_disableHudElement00);
    tolua_function(tolua_S,"isZoomCamera",tolua_LuaInterface_Tutorial_isZoomCamera00);
    tolua_function(tolua_S,"isChatBox",tolua_LuaInterface_Tutorial_isChatBox00);
    tolua_function(tolua_S,"isFoodSelected",tolua_LuaInterface_Tutorial_isFoodSelected00);
    tolua_function(tolua_S,"isFoodUsed",tolua_LuaInterface_Tutorial_isFoodUsed00);
    tolua_function(tolua_S,"isCloneDataSaved",tolua_LuaInterface_Tutorial_isCloneDataSaved00);
    tolua_function(tolua_S,"isItemsInsured",tolua_LuaInterface_Tutorial_isItemsInsured00);
    tolua_function(tolua_S,"isCloseHolocron",tolua_LuaInterface_Tutorial_isCloseHolocron00);
    tolua_function(tolua_S,"isChangeLookAtTarget",tolua_LuaInterface_Tutorial_isChangeLookAtTarget00);
    tolua_function(tolua_S,"isOpenInventory",tolua_LuaInterface_Tutorial_isOpenInventory00);
    tolua_function(tolua_S,"isCloseInventory",tolua_LuaInterface_Tutorial_isCloseInventory00);
    tolua_function(tolua_S,"getState",tolua_LuaInterface_Tutorial_getState00);
    tolua_function(tolua_S,"setState",tolua_LuaInterface_Tutorial_setState00);
    tolua_function(tolua_S,"getSubState",tolua_LuaInterface_Tutorial_getSubState00);
    tolua_function(tolua_S,"setSubState",tolua_LuaInterface_Tutorial_setSubState00);
    tolua_function(tolua_S,"enableTutorial",tolua_LuaInterface_Tutorial_enableTutorial00);
    tolua_function(tolua_S,"disableTutorial",tolua_LuaInterface_Tutorial_disableTutorial00);
    tolua_function(tolua_S,"getPlayerPosX",tolua_LuaInterface_Tutorial_getPlayerPosX00);
    tolua_function(tolua_S,"getPlayerPosY",tolua_LuaInterface_Tutorial_getPlayerPosY00);
    tolua_function(tolua_S,"getPlayerPosZ",tolua_LuaInterface_Tutorial_getPlayerPosZ00);
    tolua_function(tolua_S,"getPlayerPosToObject",tolua_LuaInterface_Tutorial_getPlayerPosToObject00);
    tolua_function(tolua_S,"enableNpcConversationEvent",tolua_LuaInterface_Tutorial_enableNpcConversationEvent00);
    tolua_function(tolua_S,"isNpcConversationStarted",tolua_LuaInterface_Tutorial_isNpcConversationStarted00);
    tolua_function(tolua_S,"isNpcConversationEnded",tolua_LuaInterface_Tutorial_isNpcConversationEnded00);
    tolua_function(tolua_S,"enableItemContainerEvent",tolua_LuaInterface_Tutorial_enableItemContainerEvent00);
    tolua_function(tolua_S,"isContainerOpen",tolua_LuaInterface_Tutorial_isContainerOpen00);
    tolua_function(tolua_S,"isContainerClosed",tolua_LuaInterface_Tutorial_isContainerClosed00);
    tolua_function(tolua_S,"isContainerEmpty",tolua_LuaInterface_Tutorial_isContainerEmpty00);
    tolua_function(tolua_S,"isItemTransferedFromContainer",tolua_LuaInterface_Tutorial_isItemTransferedFromContainer00);
    tolua_function(tolua_S,"getPlayer",tolua_LuaInterface_Tutorial_getPlayer00);
    tolua_function(tolua_S,"addQuestWeapon",tolua_LuaInterface_Tutorial_addQuestWeapon00);
    tolua_function(tolua_S,"npcStopFight",tolua_LuaInterface_Tutorial_npcStopFight00);
    tolua_function(tolua_S,"isLowHam",tolua_LuaInterface_Tutorial_isLowHam00);
    tolua_function(tolua_S,"getSkillTrainerTypeId",tolua_LuaInterface_Tutorial_getSkillTrainerTypeId00);
    tolua_function(tolua_S,"isPlayerTrained",tolua_LuaInterface_Tutorial_isPlayerTrained00);
    tolua_function(tolua_S,"setTutorialRefugeeTaunts",tolua_LuaInterface_Tutorial_setTutorialRefugeeTaunts00);
    tolua_function(tolua_S,"setTutorialCelebTaunts",tolua_LuaInterface_Tutorial_setTutorialCelebTaunts00);
    tolua_function(tolua_S,"makeCreatureAttackable",tolua_LuaInterface_Tutorial_makeCreatureAttackable00);
    tolua_function(tolua_S,"npcSendAnimation",tolua_LuaInterface_Tutorial_npcSendAnimation00);
    tolua_function(tolua_S,"getReady",tolua_LuaInterface_Tutorial_getReady00);
    tolua_function(tolua_S,"getRoom",tolua_LuaInterface_Tutorial_getRoom00);
    tolua_endmodule(tolua_S);
    tolua_cclass(tolua_S,"ObjectControllerCommandMap","ObjectControllerCommandMap","",NULL);
    tolua_beginmodule(tolua_S,"ObjectControllerCommandMap");
    tolua_function(tolua_S,"getSingletonPtr",tolua_LuaInterface_ObjectControllerCommandMap_getSingletonPtr00);
    tolua_function(tolua_S,"ScriptRegisterEvent",tolua_LuaInterface_ObjectControllerCommandMap_ScriptRegisterEvent00);
    tolua_endmodule(tolua_S);
    tolua_cclass(tolua_S,"ScriptEngine","ScriptEngine","",NULL);
    tolua_beginmodule(tolua_S,"ScriptEngine");
    tolua_function(tolua_S,"Init",tolua_LuaInterface_ScriptEngine_Init00);
    tolua_function(tolua_S,"getTutorial",tolua_LuaInterface_ScriptEngine_getTutorial00);
    tolua_endmodule(tolua_S);
    tolua_cclass(tolua_S,"ScriptSupport","ScriptSupport","",NULL);
    tolua_beginmodule(tolua_S,"ScriptSupport");
    tolua_function(tolua_S,"Instance",tolua_LuaInterface_ScriptSupport_Instance00);
    tolua_function(tolua_S,"npcCreate",tolua_LuaInterface_ScriptSupport_npcCreate00);
    tolua_function(tolua_S,"npcGetObject",tolua_LuaInterface_ScriptSupport_npcGetObject00);
    tolua_function(tolua_S,"npcSpawn",tolua_LuaInterface_ScriptSupport_npcSpawn00);
    tolua_function(tolua_S,"npcSpawnPrivate",tolua_LuaInterface_ScriptSupport_npcSpawnPrivate00);
    tolua_function(tolua_S,"npcSpawnPersistent",tolua_LuaInterface_ScriptSupport_npcSpawnPersistent00);
    tolua_function(tolua_S,"lairSpawn",tolua_LuaInterface_ScriptSupport_lairSpawn00);
    tolua_function(tolua_S,"npcDirection",tolua_LuaInterface_ScriptSupport_npcDirection00);
    tolua_function(tolua_S,"npcMove",tolua_LuaInterface_ScriptSupport_npcMove00);
    tolua_function(tolua_S,"npcMoveToZone",tolua_LuaInterface_ScriptSupport_npcMoveToZone00);
    tolua_function(tolua_S,"npcFormationPosition",tolua_LuaInterface_ScriptSupport_npcFormationPosition00);
    tolua_function(tolua_S,"npcClonePosition",tolua_LuaInterface_ScriptSupport_npcClonePosition00);
    tolua_function(tolua_S,"npcFormationMove",tolua_LuaInterface_ScriptSupport_npcFormationMove00);
    tolua_function(tolua_S,"npcFormationMoveEx",tolua_LuaInterface_ScriptSupport_npcFormationMoveEx00);
    tolua_function(tolua_S,"itemCreate",tolua_LuaInterface_ScriptSupport_itemCreate00);
    tolua_function(tolua_S,"itemPopulateInventory",tolua_LuaInterface_ScriptSupport_itemPopulateInventory00);
    tolua_function(tolua_S,"objectIsReady",tolua_LuaInterface_ScriptSupport_objectIsReady00);
    tolua_function(tolua_S,"getObject",tolua_LuaInterface_ScriptSupport_getObject00);
    tolua_function(tolua_S,"eraseObject",tolua_LuaInterface_ScriptSupport_eraseObject00);
    tolua_function(tolua_S,"getZoneId",tolua_LuaInterface_ScriptSupport_getZoneId00);
    tolua_function(tolua_S,"enableInstance",tolua_LuaInterface_ScriptSupport_enableInstance00);
    tolua_function(tolua_S,"isInstance",tolua_LuaInterface_ScriptSupport_isInstance00);
    tolua_function(tolua_S,"scriptPlayMusic",tolua_LuaInterface_ScriptSupport_scriptPlayMusic00);
    tolua_function(tolua_S,"getTarget",tolua_LuaInterface_ScriptSupport_getTarget00);
    tolua_function(tolua_S,"getParentOfTarget",tolua_LuaInterface_ScriptSupport_getParentOfTarget00);
    tolua_function(tolua_S,"sendFlyText",tolua_LuaInterface_ScriptSupport_sendFlyText00);
    tolua_function(tolua_S,"scriptSystemMessage",tolua_LuaInterface_ScriptSupport_scriptSystemMessage00);
    tolua_function(tolua_S,"setPlayerPosition",tolua_LuaInterface_ScriptSupport_setPlayerPosition00);
    tolua_function(tolua_S,"npcInCombat",tolua_LuaInterface_ScriptSupport_npcInCombat00);
    tolua_function(tolua_S,"npcIsDead",tolua_LuaInterface_ScriptSupport_npcIsDead00);
    tolua_function(tolua_S,"npcKill",tolua_LuaInterface_ScriptSupport_npcKill00);
    tolua_function(tolua_S,"npcTestDir",tolua_LuaInterface_ScriptSupport_npcTestDir00);
    tolua_endmodule(tolua_S);
    tolua_endmodule(tolua_S);
    return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
TOLUA_API int luaopen_LuaInterface (lua_State* tolua_S) {
    return tolua_LuaInterface_open(tolua_S);
}
#endif

