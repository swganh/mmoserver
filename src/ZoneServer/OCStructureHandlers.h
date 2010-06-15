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

#ifndef ZONESERVER_OCSTRUCTUREHANDLERS_H_
#define ZONESERVER_OCSTRUCTUREHANDLERS_H_

class Message;
class Object;
class ObjectControllerCmdProperties;

/// This command is used to move items about a structure.
/**
 * This command is invoked by the client to move items around in a structure.
 *
 * The client enters the message in the following format:
 *   /moveFurniture <FORWARD/BACK/UP/DOWN> <distance>
 *
 * @param object The object moving the furniture (always a PlayerObject).
 * @param target The targeted item is the one being moved.
 * @param message The message from the client requesting this command.
 * @param cmd_properties Contextual information for use during processing this command.
 */
bool HandleMoveFurniture(Object* object, Object* target, Message* message, ObjectControllerCmdProperties* cmdProperties);

#endif  // ZONESERVER_OCSTRUCTUREHANDLERS_H_
