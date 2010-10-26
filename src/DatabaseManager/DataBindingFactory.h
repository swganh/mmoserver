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

#ifndef ANH_DATABASEMANAGER_DATABINDINGFACTORY_H
#define ANH_DATABASEMANAGER_DATABINDINGFACTORY_H

#include <cstdint>

#include <boost/noncopyable.hpp>
#include <boost/pool/pool.hpp>

#include "DatabaseManager/declspec.h"

// Win32 complains about stl during linkage, disable the warning.
#ifdef _WIN32
#pragma warning (push)
#pragma warning (disable : 4251 4275)
#endif

class DataBinding;
struct DataField;

class DBMANAGER_API DataBindingFactory : private boost::noncopyable {
public:
    DataBindingFactory();
    ~DataBindingFactory();

    DataBinding* CreateDataBinding(uint16_t fieldCount);
    void DestroyDataBinding(DataBinding* binding);

    bool releasePoolMemory() {
        return(binding_pool_.release_memory());
    }

private:
    boost::pool<boost::default_user_allocator_malloc_free>	binding_pool_;
};

// Re-enable the warning.
#ifdef _WIN32
#pragma warning (pop)
#endif

#endif // ANH_DATABASEMANAGER_DATABINDINGFACTORY_H
