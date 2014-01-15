// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "../visitor_interface.h"

#include <vector>
#include <list>

namespace swganh
{
namespace tre
{

	class SlotArrangementVisitor : public VisitorInterface
	{
	public:
		static const VisitorType Type = SLOT_ARRANGEMENT_VISITOR;

		/**
			@brief returns the InterpreterType associated with this Interpreter
		*/
		virtual VisitorType getType() { return SLOT_ARRANGEMENT_VISITOR; }

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

		//const std::vector<std::string>& slots_occupied(size_t id) { return combinations_[id]; };

		std::list<std::vector<std::string>>::const_iterator begin() {return combinations_.cbegin();}
		std::list<std::vector<std::string>>::const_iterator end() {return combinations_.cend();}

	private:
		std::list<std::vector<std::string>> combinations_;

	};
}
}
