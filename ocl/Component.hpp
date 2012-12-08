/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:35:28 CEST 2008  Component.hpp

                        Component.hpp -  description
                           -------------------
    begin                : Thu July 03 2008
    copyright            : (C) 2008 Peter Soetens
    email                : peter.soetens@fmtc.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file Component.hpp
 *
 * This file contains the macros and definitions to create dynamically
 * loadable components. You need to include this header and use one
 * of the macros if you wish to make a run-time loadable component.
 */

#ifndef OCL_COMPONENT_HPP
#define OCL_COMPONENT_HPP

// functionality of ocl/Component.hpp is in rtt/Component.hpp now, just forward
#include <rtt/Component.hpp>

// In order to silent client code which assumes we declare this namespace (see bug #999)
namespace OCL {}

#endif
