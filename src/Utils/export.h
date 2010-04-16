/**
 * SWG:ANH - Star Wars Galaxies - A New Hope
 * An Open-Source Massively Multiplayer Server
 *
 * Copyright (c) 2006 - 2010 The SWG:ANH Team <http://www.swganh.com>
 */

// *********************************************************************
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// To read the license please visit http://www.gnu.org/copyleft/gpl.html
// *********************************************************************

#ifndef SRC_UTILS_EXPORT_H_
#define SRC_UTILS_EXPORT_H_

// export.h: Export definitions. Modeled after osg/Export.

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#   ifdef UTILS_EXPORTS
#      define ANH_EXPORT __declspec(dllexport)
#   else
#      define ANH_EXPORT __declspec(dllimport)
#   endif 
#else
#   define ANH_EXPORT
#endif

#endif  // SRC_UTILS_EXPORT_H_
