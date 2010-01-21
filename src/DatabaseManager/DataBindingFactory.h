/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABINDINGFACTORY_H
#define ANH_DATABASEMANAGER_DATABINDINGFACTORY_H


#include "Utils/typedefs.h"
#include <boost/pool/pool.hpp>


//======================================================================================================================

class DataBinding;
class DataField;

//======================================================================================================================

class DataBindingFactory
{
	public:

		DataBindingFactory(void);
		~DataBindingFactory(void);

		void			Startup(void);
		void			Shutdown(void);

		DataBinding*	CreateDataBinding(uint16 fieldCount);
		void			DestroyDataBinding(DataBinding* binding);

		bool			releasePoolMemory(){ return(mDataBindingPool.release_memory()); }

	private:

		boost::pool<boost::default_user_allocator_malloc_free>	mDataBindingPool;
};

//======================================================================================================================

#endif // ANH_DATABASEMANAGER_DATABINDINGFACTORY_H






