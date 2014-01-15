// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "../visitor_interface.h"

#include <map>
#include <string>
#include <list>
#include <vector>
#include <cstdint>
#include <boost/any.hpp>

namespace swganh
{
namespace tre
{
	class DatatableVisitor : public VisitorInterface
	{
	public:
		static const VisitorType Type = DATATABLE_VISITOR;

		/**
			@brief interprets a IFF::FileNode associated with this visitor.
			This should only be called by the IFFFile code.
		*/
		virtual void visit_data(uint32_t depth, std::string name, uint32_t size, swganh::ByteBuffer& data);

		/**
			@brief interprets a IFF::FolderNode associated with this visitor.
			This should only be called by the IFFFile code.
		*/
		virtual void visit_folder(uint32_t depth, std::string name, uint32_t size);

		class DATA_ROW
		{
		public:

			template <typename T>
			T GetValue(int column_id) {
				return boost::any_cast<T>(columns.at(column_id));
			}

			std::vector<boost::any> columns;
		};

		const std::vector<std::string>& column_names() { return column_names_; }
		const std::vector<char>& column_types() { return column_types_; }
					
		std::list<DATA_ROW>::iterator begin_itr() { return rows_.begin(); }
		std::list<DATA_ROW>::iterator end_itr() { return rows_.end(); }

	private:
		void _handle0001COLS(swganh::ByteBuffer& buf);
		void _handleTYPE(swganh::ByteBuffer& buf);
		void _handleROWS(swganh::ByteBuffer& buf);

		std::vector<char> column_types_;
		std::vector<std::string> column_names_;
		std::list<DATA_ROW> rows_;
	};
}
}
