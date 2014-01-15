// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "../visitor_interface.h"
#include <vector>
#include <map>
#include <functional>
#include <cstdint>
#include <glm/glm.hpp>

namespace swganh
{
	class ByteBuffer;
}

namespace swganh
{
namespace tre
{
	class ResourceManager;
	class VisitorInterface;
	
	class PobVisitor : public VisitorInterface
	{
	public:
		static const VisitorType Type = POB_VISITOR;

		typedef std::function<void(PobVisitor*, swganh::ByteBuffer& buffer)> NodeFunctor;
		typedef std::map<std::string, NodeFunctor> NodeNameIndex;
		typedef std::map<std::string, NodeFunctor>::iterator NodeNameIndexIterator;

		/**
			@brief Constructor for a pob interpreter
		*/
		PobVisitor();

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

		/**
			@brief A portal object used inside the POB files.
		*/
		class Portal
		{
		public:
			std::vector<glm::vec3> vertices;
		};

		/**
			@brief A Link between a Cell and a Portal Object
		*/
		class Link
		{
		public:
			uint32_t portal_id; //portal id
					
			char unkFlag2;
			uint32_t dst_cellid; //destination cell id

			//BELOW HERE IS LIKELY DOOR STUFF WE ARE NOT CONCERNED WITH
			std::string doorname;

			glm::vec3 unk1; //Seems like an inefficient bitmask
			float unk2; //scale?

			glm::vec3 unk3; //Seems like an inefficient bitmask
			float unk4; //Scale?

			glm::vec3 unk5; //Seems like an inefficient bitmask
			float unk6; //Scale?

		};

		/**
			@A simple triangle class that references different vertices.
		*/
		class triangle
		{
		public:
			uint32_t a;
			uint32_t b;
			uint32_t c;
		};

		/**
			@brief A cell class that has a certain shape, id, mesh, and collision
		*/
		class Cell
		{
		public:
			//0005DATA
			char unkFlag2;
			std::string name;
			char unkFlag3;
			std::string mesh;
			char unkFlag4;
			std::string collision;

			//0000VERT
			std::vector<glm::vec3> vertices;

			//INDX
			std::vector<triangle> triangles;

			//PRTL0004
			std::vector<Link> links;
		};

		/**
			@return the number of cells in this pob file
		*/
		unsigned int cell_count() { return cells_.size(); }
				
		/**
			@return the number of portals in this pob file.
		*/
		unsigned int portal_count() { return portals_.size(); }

		/**
			@brief Fetches a portal object
			@param id the id of the portal to fetch
			@return the fetched portal
		*/
		const Portal& getPortal(unsigned int id) { return portals_[id]; }
				
		/**
			@brief Fetches a cell object
			@param id the id of the cell to fetch
			@return the fetched cell
		*/
		const Cell& getCell(unsigned int id) { return cells_[id]; }

		/**
			@brief a simple output function for debugging values.
		*/
		void debug();

	private:

		//Internal Node Handling functions
		static void _handle0003DATA(PobVisitor*, swganh::ByteBuffer& data);
		static void _handlePRTL(PobVisitor*, swganh::ByteBuffer& data);
		static void _handle0005DATA(PobVisitor*, swganh::ByteBuffer& data);
		static void _handle0000VERT(PobVisitor*, swganh::ByteBuffer& data);
		static void _handleINDX(PobVisitor*, swganh::ByteBuffer& data);
		static void _handlePRTL0004(PobVisitor*, swganh::ByteBuffer& data);

		//Internal Containers for portals and cells
		std::vector<Portal> portals_;
		std::vector<Cell> cells_;

		static NodeNameIndex nameLookup_;
	};
}
}
