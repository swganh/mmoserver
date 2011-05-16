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

#ifndef SRC_ZONESERVER_SOCIALCHATTYPES_H_
#define SRC_ZONESERVER_SOCIALCHATTYPES_H_

#include <cstdint>

/**
 * This Enum is used by spatial chat to indicate the manner in which a given
 * body of speach is delivered. This is reflected in the prose speach that is
 * displayed as well as the chat bubble.
 */

enum SocialChatType :
uint16_t {
    kSocialChatNone = 0,
    kSocialChatSay,
    kSocialChatAdd,
    kSocialChatAdmit,
    kSocialChatAnnounce,
    kSocialChatAnswer,
    kSocialChatArgue,
    kSocialChatAssert,
    kSocialChatAvow,
    kSocialChatBabble,
    kSocialChatBeg,
    kSocialChatBellow,
    kSocialChatBlab,
    kSocialChatBleat,
    kSocialChatBlurt,
    kSocialChatBoast,
    kSocialChatBrag,
    kSocialChatCarol,
    kSocialChatChant,
    kSocialChatChat,
    kSocialChatChatter,
    kSocialChatCommand,
    kSocialChatComplain,
    kSocialChatConcede,
    kSocialChatConclude,
    kSocialChatConfess,
    kSocialChatConfide,
    kSocialChatCoo,
    kSocialChatDebate,
    kSocialChatDeclare,
    kSocialChatDecree,
    kSocialChatDemand,
    kSocialChatDescribe,
    kSocialChatDisclose,
    kSocialChatDivulge,
    kSocialChatDrawl,
    kSocialChatDrivel,
    kSocialChatDrone,
    kSocialChatEmote,
    kSocialChatEulogize,
    kSocialChatExclaim,
    kSocialChatExplain,
    kSocialChatForetell,
    kSocialChatGab,
    kSocialChatGossip,
    kSocialChatHowl,
    kSocialChatHuff,
    kSocialChatHypothesize,
    kSocialChatIndicate,
    kSocialChatInquire,
    kSocialChatInterject,
    kSocialChatInterrupt,
    kSocialChatIntone,
    kSocialChatJabber,
    kSocialChatJaw,
    kSocialChatJest,
    kSocialChatLecture,
    kSocialChatLisp,
    kSocialChatMoan,
    kSocialChatMouth,
    kSocialChatMumble,
    kSocialChatMuse,
    kSocialChatNote,
    kSocialChatParrot,
    kSocialChatPlead,
    kSocialChatPrattle,
    kSocialChatPreach,
    kSocialChatProclaim,
    kSocialChatPromise,
    kSocialChatPronounce,
    kSocialChatProphesize,
    kSocialChatPropose,
    kSocialChatQuote,
    kSocialChatRamble,
    kSocialChatRap,
    kSocialChatRecite,
    kSocialChatRespond,
    kSocialChatRetort,
    kSocialChatRiddle,
    kSocialChatScream,
    kSocialChatShout,
    kSocialChatShrill,
    kSocialChatSing,
    kSocialChatSlur,
    kSocialChatSocial,
    kSocialChatState,
    kSocialChatStutter,
    kSocialChatSuppose,
    kSocialChatSurmise,
    kSocialChatSwear,
    kSocialChatTattle,
    kSocialChatTheorize,
    kSocialChatThink,
    kSocialChatThreaten,
    kSocialChatUtter,
    kSocialChatVent,
    kSocialChatVow,
    kSocialChatWail,
    kSocialChatWhine,
    kSocialChatWhisper,
    kSocialChatYack,
    kSocialChatYell,
    kSocialChatYelp
};

#endif  // SRC_ZONESERVER_SOCIALCHATTYPES_H_
