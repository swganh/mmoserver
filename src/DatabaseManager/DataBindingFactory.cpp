/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "DataBindingFactory.h"
#include "DataBinding.h"


//======================================================================================================================

DataBindingFactory::DataBindingFactory(void) :
mDataBindingPool(sizeof(DataBinding))
{

}

//======================================================================================================================

DataBindingFactory::~DataBindingFactory(void)
{

}

//======================================================================================================================

void DataBindingFactory::Startup(void)
{
	
}

//======================================================================================================================

void DataBindingFactory::Shutdown(void)
{

}

//======================================================================================================================

DataBinding* DataBindingFactory::CreateDataBinding(uint16 fieldCount)
{
	return(new (mDataBindingPool.ordered_malloc()) DataBinding(fieldCount));
}

//======================================================================================================================

void DataBindingFactory::DestroyDataBinding(DataBinding* binding)
{
	mDataBindingPool.ordered_free(binding);
}

//======================================================================================================================





