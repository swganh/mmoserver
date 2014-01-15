// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "../visitor_interface.h"

#include <map>
#include <unordered_map>
#include <set>
#include <functional>
#include <cstdint>
#include <memory>
#include <boost/any.hpp>

namespace swganh
{
namespace resource
{
	class ResourceManagerInterface;
}
}

namespace swganh
{
namespace tre
{
	class ObjectVisitor;
	class ResourceManager;
	//End Forward Declarations

	//Begin Typedefs
	typedef std::function<void(ObjectVisitor*, std::string&, swganh::ByteBuffer&)> AttributeFunctor;
	typedef std::map<std::string, AttributeFunctor> AttributeHandlerIndex;
	typedef std::map<std::string, AttributeFunctor>::const_iterator AttributeHandlerIndexIterator;
	typedef std::map<std::string, std::shared_ptr<boost::any>> AttributeMap;
	//End Typedefs

	/**
		@brief An IFFVisitor for object iff files.
	*/
	class ObjectVisitor : public VisitorInterface
	{
	public:
		static const VisitorType Type = OIFF_VISITOR;

		/**
			@brief Constructor for an object iff visitor
		*/
		ObjectVisitor();
				
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

		/**
			@brief An internal ClientString structure. This could later be moved outside this class.
			It is merely here now for completeness.
		*/
		class ClientString
		{
		public:
			std::string file;
			std::string entry;
		};

		/**
			@brief returns an attribute loaded from the object iff associated with this interpreter.

			@param key the key to lookup

			@return a boost::any casted shared_ptr<T> that was requested
		*/
		template <class T> T attribute(const std::string& key);
		
		bool has_attribute(const std::string& key);

		uint32_t attribute_uint32(std::string& key);
		float attribute_float(std::string& key);
		bool attribute_bool(std::string& key);


		/**
			@brief A function which causes this OIFFInterpreter to load parent files it needs and then copy
			their values downstream

			@param f the filemanager instance for retriving the necessary files.
		*/
		void load_aggregate_data(swganh::tre::ResourceManager* f);

	private:
		//Internal Index used to link the handlers with the attributes
		static AttributeHandlerIndex attributeHandler_;

		//Internal Datatype handling functions used to parse the object files
		static void _handleUnhandled(ObjectVisitor* dst, std::string& name, swganh::ByteBuffer& buf);
		static void _handleClientString(ObjectVisitor* dst, std::string& name, swganh::ByteBuffer& buf);
		static void _handleString(ObjectVisitor* dst, std::string& name, swganh::ByteBuffer& buf);
		static void _handleInt(ObjectVisitor* dst, std::string& name, swganh::ByteBuffer& buf);
		static void _handleFloat(ObjectVisitor* dst, std::string& name, swganh::ByteBuffer& buf);
		static void _handleBool(ObjectVisitor* dst, std::string& name, swganh::ByteBuffer& buf);

		//Internal Node Handling functions
		void _handleXXXX(swganh::ByteBuffer& buf);
		void _handleDERVXXXX(swganh::ByteBuffer& buf);

		//Attributes this object iff might have
		AttributeMap attributes_;

		//Parent files this object iff might have
		std::set<std::string> parentFiles;

		//Stored for determing if we've already loaded our aggregate information.
		bool has_aggregate_;
		bool loaded_reference_;
	};
}
}

#include "object_visitor-intl.h"
