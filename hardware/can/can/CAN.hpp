/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:20 CET 2004  CAN.hpp

                        CAN.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

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

#include <rtt/RTT.hpp>
#include "CANBus.hpp"
#include "CANBusInterface.hpp"
#include "CANConfigurator.hpp"
#include "CANControllerInterface.hpp"
#include "CANDeviceInterface.hpp"
#include "CANDeviceRegistrator.hpp"
#include "CANMessage.hpp"
#include "CANBus.hpp"
#include "CANPieController.hpp"
#include "CANRequest.hpp"
#include "NodeGuard.hpp"
#include "PCANController.hpp"
#include "SyncWriter.hpp"

namespace RTT
{
    /**
     * @brief CAN Message Abstraction Layer
     *
     * This package provides a C++ CANOpen message and controller layer.
     * A controller based on the canpie package is provided.
     *
     */
    namespace CAN {}
}

