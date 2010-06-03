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
#include <cstdint>
#ifdef _MSC_VER
#include <regex>  // NOLINT
#else
#endif

#include <boost/regex.hpp>  // NOLINT

#include "MedicHandlerHelpers.h"
#include "ObjectController.h"
#include "MedicManager.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "ForageManager.h"

#ifdef WIN32
using ::std::regex;
using ::std::smatch;
using ::std::regex_search;
using ::std::sregex_token_iterator;
#else
using ::boost::regex;
using ::boost::smatch;
using ::boost::regex_search;
using ::boost::sregex_token_iterator;
#endif

bool			MedicHandlerHelpers::mInsFlag = false;
MedicHandlerHelpers*	MedicHandlerHelpers::mSingleton = NULL;

MedicHandlerHelpers::MedicHandlerHelpers(MessageDispatch* dispatch)
{
    Dispatch = dispatch;
}
MedicHandlerHelpers::~MedicHandlerHelpers()
{
}

std::string MedicHandlerHelpers::handleMessage(Message* message, std::string regexPattern)
{
	// Read the message out of the packet.
	string tmp;
	message->getStringUnicode16(tmp);

	// If the string has no length the message is ill-formatted, send the
	// proper format to the client.
	if (!tmp.getLength())
		return "";

	// Convert the string to an ansi string for ease with the regex.
	tmp.convert(BSTRType_ANSI);
	std::string input_string(tmp.getAnsi());

	static const regex pattern(regexPattern);
	smatch result;

	regex_search(input_string, result, pattern);
  
	// Gather the results of the pattern for validation and use.
	std::string messageType(result[1]);
	if (messageType.length() > 0)
	{
		return messageType;
	}
	return "";
}