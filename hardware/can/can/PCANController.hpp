/***************************************************************************
  tag: Klaas Gadeyne  Mon Jul 24 14:11:20 CET 2006  PCANController.hpp

                        PCANController.hpp -  description
                           -------------------
  Non-realtime CAN driver using the PCAN linux driver.

    begin                : Mon Oct 17  2006
    copyright            : (C) 2006 FMTC
    email                : firstname lastname at fmtc be

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

#ifndef PCAN_CONTROLLER_HPP
#define PCAN_CONTROLLER_HPP

#include "CAN.hpp"
#include "CANControllerInterface.hpp"
#include "CANBusInterface.hpp"
#include "CANMessage.hpp"

#include <rtt/NonPeriodicActivity.hpp>

#include <libpcan.h>

namespace RTT
{
  namespace CAN
  {
    /**
     * A Controller which interacts with the PCAN linux driver to
     * address the physical controller.  This controller uses the
     * NON-REALTIME CAN driver from Peak Systems (See
     * http://www.peak-system.com/linux/) and should support all their
     * boards, although it's only tested with their PCI card.
     */
    class PCANController
      : public CANControllerInterface
      , public NonPeriodicActivity
    {
    public:
      /**
       * @brief Create a PCAN Controller
       * @param priority the priority of the activity that mirrors the
       * physical and software CAN bus
       * @param period the priority of the activity that mirrors the
       * physical and software CAN bus
       * @param minor minor number of the CAN device node.  E.g. if you
       * are using /dev/pcan1, minor is 1
       * @param bitrate bitrate of the CAN bus.  Possible values are
       * ranging from CAN_BAUD_5K (5 kbits/s) to CAN_BAUD_1M (1 Mbit/s).  See
       * libpcan.h for an exhaustive list.  Defaults to CAN_BAUD_500K.
       * @param CANMsgType Standard or Extended frames
       * CAN_INIT_TYPE_EX CAN_INIT_TYPE_ST.  Defaults to CAN_INIT_TYPE_EX
       * FIXME Klaas Gadeyne.  Does CAN_INIT_TYPE_ST or CAN_INIT_TYPE_EX
       * means the controller can either send standard or extended
       * frames here???
       * @bug This constructor should throw an exception if
       * LINUX_CAN_Open returns NULL
       * @bug The node id of the controller is always 0 currently
       */
      PCANController(int priority,
		     unsigned int minor=0,
		     WORD bitrate=CAN_BAUD_500K,
		     int CANMsgType=CAN_INIT_TYPE_EX);

      virtual ~PCANController();

      // Redefine PeriodicActivity methods
      bool initialize();
      void loop();
      bool breakLoop();
      void finalize();

      // Redefine CANControllerInterface
      virtual void addBus( unsigned int chan, CANBusInterface* _bus);
      virtual void process(const CANMessage* msg);
      virtual unsigned int nodeId() const;
      bool readFromBuffer( CANMessage& msg);
      bool writeToBuffer(const CANMessage * msg);

      // Ask status
      DWORD status() const;

      // Return pcan device handle
      HANDLE handle() const;

    protected:
      // Maximum number of CAN channels
      // KG set this to one, as I didn't know by heart and only needed one...
#define PCAN_CHANNEL_MAX 1
      static PCANController* controller[PCAN_CHANNEL_MAX];

      HANDLE _handle; // Handle to device
      DWORD _status; // Status of the CAN Controller
      WORD _bitrate;
      int _CANMsgType;

      int _channel;

      CANBusInterface * _bus;
      CANMessage _CANMsg;

      unsigned int total_recv;
      unsigned int total_trns;
      unsigned int failed_recv;
      unsigned int failed_trns;

      bool exit;
    };
  }
}

#endif


