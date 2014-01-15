// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

namespace swganh
{
namespace tre
{
	//Defines Interpreter types for telling apart different
	//Visitor that have been cast to the IFF_Visitor base class.
	enum VisitorType
	{
		IFF_VISITOR, //BASE CLASS

		POB_VISITOR,
		TRN_VISITOR,
		WS_VISITOR,
		OIFF_VISITOR,
		SLOT_ARRANGEMENT_VISITOR,
		SLOT_DESCRIPTOR_VISITOR,
		SLOT_DEFINITION_VISITOR,
		DATATABLE_VISITOR,
		LAY_VISITOR
	};
}
}
