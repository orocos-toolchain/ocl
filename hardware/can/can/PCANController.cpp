/***************************************************************************
  tag: Klaas Gadeyne  Mon Jan 10 15:59:17 CET 2005  PCANController.cpp 

                        PCANController.cpp -  description
                           -------------------
    begin                : 16 October 2006
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
 
#include "PCANController.hpp"
#include <fcntl.h>    // O_RDWR
#include <rtt/Logger.hpp>

namespace RTT
{
    namespace CAN
    {
        PCANController::PCANController(int priority, Seconds period, unsigned int minor,
                                       WORD bitrate, int CANMsgType) 
            : PeriodicActivity(ORO_SCHED_OTHER, priority,period), _handle(NULL),
              _status(CAN_ERR_OK), _bitrate(bitrate),
              _CANMsgType(CANMsgType),_channel(0),
              total_recv(0), total_trns(0), failed_recv(0), failed_trns(0)
        {
            Logger::In in("PCANController");
      
            // This is a Non-Realtime driver!!
            log(Info) <<  "This is NOT a real-time CAN driver." << endlog();
      
            char DeviceName[ 12 ]; // don't specify /dev/pcan100 or more, you moron
            snprintf(DeviceName,12, "/dev/pcan%d", minor );
            log(Info) << "Trying to open " << DeviceName << endlog();
            _handle = LINUX_CAN_Open(DeviceName, O_RDWR);
            if (_handle != NULL){ 
                // Reset status
                CAN_Status(_handle);
                // Get version info
                char version_string[VERSIONSTRING_LEN];
                DWORD err = CAN_VersionInfo(_handle, version_string);
                if (!err)
                    log(Info) <<  "Driver Version = " << version_string << endlog();
                else 
                    log(Warning) << "Error getting driver version info" << endlog();
            }
            else {
                log(Error) << "Error opening " << DeviceName << endlog();
                // Add assert here?
            }
        }
            
        PCANController::~PCANController()
        {
            Logger::In in("PCANController");
            log(Info) << "PCAN Controller Statistics :"<<endlog();
            log(Info) << " Total Received    : "<<total_recv<< ".  Failed to Receive : "<< failed_recv<< endlog();
            log(Info) << " Total Transmitted : "<<total_trns<< ".  Failed to Transmit : "<< failed_trns<< endlog();
        }

        HANDLE PCANController::handle() const {
            return _handle;
        }

        bool PCANController::initialize() {
            Logger::In in("PCANController");
            _status = CAN_Init(_handle, _bitrate, _CANMsgType);
            if (_status != CAN_ERR_OK)
                log(Warning) << "Error initializing driver, status = " << status() << endlog();
            //  else 
            // Happens in a realtime thread
            // log(Info) <<  "Driver initialized..." << endlog();
            return (_status == CAN_ERR_OK);
        }

        void PCANController::step() 
        {
            while ( this->readFromBuffer(_CANMsg) ){
                _CANMsg.origin = this;
                _bus->write(&_CANMsg); // we own _CANMsg;
            }
        }
        
        void PCANController::finalize() {
            Logger::In in("PCANController");
            _status = CAN_Close(_handle);
            if (_status != CAN_ERR_OK)
                log(Error) << "Error shutting down driver, status = " << status() << endlog();
            // else 
            // Happens in a realtime thread
            // log(Info) <<  "Can_Close succeeded..." << endlog();
        }

        void PCANController::addBus( unsigned int chan, CANBusInterface* bus)
        {
            _channel = chan; _bus = bus;
            controller[_channel] = this;
            bus->setController( this );
        }

        void PCANController::process(const CANMessage* msg)
        {
            Logger::In in("PCANController");
            // Construct message
            TPCANMsg pcan_msg;
            for (unsigned int i=0; i < 8; i++){
                pcan_msg.DATA[i]=msg->getData(i);
            }
            // The data length code (Bit 0 - Bit 3) contains the number of
            // data bytes which are transmitted by a message. The possible
            // value range for the data length code is from 0 to 8
            // (bytes).
            pcan_msg.LEN=msg->getDLC();
        
            // EXTENDED OR STANDARD Msg?
            if (msg->isExtended()){
                pcan_msg.MSGTYPE = MSGTYPE_EXTENDED;
                pcan_msg.ID=(DWORD) msg->getExtId();
                // FIXME: Check this (see doxygen class info) 
                // if (_CANMsgType == CAN_INIT_TYPE_ST)
                // log(Warning) << "Trying to process Extended message, but _CANMsgType set to Standard!" << endlog();
            }
            else{ // if (msg->isStandard())
                pcan_msg.MSGTYPE = MSGTYPE_STANDARD;
                pcan_msg.ID=(DWORD) msg->getStdId();
                /*
                  if (_CANMsgType == CAN_INIT_TYPE_EX)
                  log(Warning) << "Trying to process Standard message, but
                  _CANMsgType set to Extended!" << endlog();
                */
            }
            // Remote Transmission request?
            if (msg->isRemote())
                pcan_msg.MSGTYPE |= MSGTYPE_RTR;

            // Send the message (non-blocking)
            // _status = LINUX_CAN_Write_Timeout(_handle,&pcan_msg,0);
            /* KG: This returns 0x080, transmit queue full, even when
               sending only 1 message and initializing the controller
            */
            // Send the message (blocking and hence non-realtime)
            _status = CAN_Write(_handle,&pcan_msg);
            if (_status == CAN_ERR_OK)
                ++total_trns;
            else{
                ++failed_trns;
                // Happens in a real-time thread
                // log(Error) << "Error sending CAN Message, _status = " << status() << endlog();
            }
        }

        unsigned int PCANController::nodeId() const
        {
            return 0;
        }

        DWORD PCANController::status() const
        {
            return _status;
        }

        bool PCANController::readFromBuffer( CANMessage& msg)
        {
            Logger::In in("PCANController");
            TPCANRdMsg pcan_msg;
            if ( (_status = LINUX_CAN_Read_Timeout(_handle, &pcan_msg, 0) ) == CAN_ERR_OK ){
                msg.clear();
                // DLC
                msg.setDLC(pcan_msg.Msg.LEN);
                // REMOTE TRANMISSION REQUEST?
                if ((pcan_msg.Msg.MSGTYPE & MSGTYPE_RTR) == MSGTYPE_RTR)
                    msg.setRemote();
                // ID (and type)
                if ((pcan_msg.Msg.MSGTYPE & MSGTYPE_EXTENDED) == MSGTYPE_EXTENDED)
                    {
                        msg.setExtId((unsigned int) pcan_msg.Msg.ID);
                        // Happens in rtt
                        //	    if (_CANMsgType == CAN_INIT_TYPE_ST)
                        // log(Warning) << "Trying to read Extended message, but _CANMsgType set to Standard!" << endlog();
                    }
                else // rx_event.msg.ext == CYGNUM_CAN_ID_STD
                    msg.setStdId((unsigned int) pcan_msg.Msg.ID);
                // DATA
                for (unsigned int i=0; i < 8; i++){
                    msg.setData(i,pcan_msg.Msg.DATA[i]);
                }
#if 0 // Some debugging code, printing each received CAN message
                log(Debug) << "Receiving CAN Message: Id=";
                if (msg.isStandard())
                    log(Debug) << msg.getStdId();
                else
                    log(Debug) << msg.getExtId();
                log(Debug) << ", DLC=" << msg.getDLC() << ", DATA = ";
                for (unsigned int i=0; i < 8; i++){ 
                    log(Debug) << (unsigned int) msg.getData(i) << " ";
                }
                log(Debug) << endlog();
#endif
                // set origin
                msg.origin = this;
                ++total_recv;
            }
            else{
                if (_status == CAN_ERR_QRCVEMPTY){ // Nothing in queue:
                    // diag_printf("PCANController::readFromBuffer(): Nothing in the FIFO\n!");
                }
                else { // Something else went wrong ...
                    // log(Error) << "Error readFromBuffer():  Error code = " << _status << endlog();
                    ++failed_recv;
                }
            }
            return (_status == CAN_ERR_OK);
        }

        PCANController* PCANController::controller[PCAN_CHANNEL_MAX];
    }
  
}
