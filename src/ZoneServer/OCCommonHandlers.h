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

// @NOTE: This is a tmp file for experimentation purposes only. These handlers
// should be placed in a more appropriate service before merging this branch.

#ifndef SRC_ZONESERVER_OCCOMMONHANDLERS_H_
#define SRC_ZONESERVER_OCCOMMONHANDLERS_H_


class Message;
class Object;
class ObjectControllerCmdProperties;

/// This command is used to move items about a structure.
/**
 * This command is invoked by the client to gain a temporary burst of speed.
 *
 * The client enters the message in the following format:
 *   /burstRun
 *
 * @param object The object attempting to burst run.
 * @param target No target is set for this command, this value is always a nullptr.
 * @param message The message from the client requesting this command.
 * @param cmd_properties Contextual information for use during processing this command.
 */
bool HandleBurstRun(Object* object, Object* target, Message* message, ObjectControllerCmdProperties* cmdProperties);

#endif  // SRC_ZONESERVER_OCCOMMONHANDLERS_H_
