// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "anh/tre/visitors/visitor_interface.h"

#include <vector>
#include <map>
#include <stack>

namespace swganh
{
namespace tre
{
	class Fractal;
	class Layer;
	class ContainerLayer;
	struct TrnHeader;

	typedef std::map<uint32_t,Fractal*> FractalMap;

	class TerrainVisitor : public VisitorInterface
	{
	public:
		static const VisitorType Type = TRN_VISITOR;

		TerrainVisitor();
		~TerrainVisitor();

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

		TrnHeader* GetHeader() { return header; }
		FractalMap& GetFractals() { return fractals_; }
		std::vector<ContainerLayer*>& GetLayers() { return layers_; }

	private:

		//This is working data.
		Fractal* working_fractal_;
		std::stack<std::pair<Layer*, uint32_t>> layer_stack_;
		Layer* working_layer_;

		//This is actual persistance data
		TrnHeader* header;
		FractalMap fractals_;
		std::vector<ContainerLayer*> layers_;
	};
}
}
