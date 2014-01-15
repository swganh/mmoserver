// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "../visitor_interface.h"

#include <cstdint>
#include <vector>

namespace swganh
{
namespace tre
{
	class SlotDescriptorVisitor : public VisitorInterface
	{
	public:
		static const VisitorType Type = SLOT_DESCRIPTOR_VISITOR;

		/**
			@brief interprets a IFF::FileNode associated with this interpreter.
			This should only be called by the IFFFile code.
		*/
		virtual void visit_data(uint32_t depth, std::string name, uint32_t size, swganh::ByteBuffer& data);

		/**
			@brief interprets a IFF::FolderNode associated with this interpreter.
			This should only be called by the IFFFile code.
		*/
		virtual void visit_folder(uint32_t depth, std::string name, uint32_t size);

		size_t available_count() {return slots_available.size();}
		std::string& slot(size_t id) {return slots_available[id];}

	private:
		std::vector<std::string> slots_available;
	};
}
}
