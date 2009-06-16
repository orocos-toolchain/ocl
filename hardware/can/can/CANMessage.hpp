/***************************************************************************
 tag: Peter Soetens  Mon Jan 19 14:11:20 CET 2004  CANMessage.hpp

 CANMessage.hpp -  description
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

#ifndef CANMESSAGE_HPP
#define CANMESSAGE_HPP

#include "cpstruct.h"
#include "cpmacro.h"
#include "CANDeviceInterface.hpp"
#include <string.h>

#define fillData(data, d0, d1, d2 ,d3, d4, d5, d6, d7) do { \
        data[0] = d0; data[1] = d1; data[2] = d2; data[3] = d3; \
        data[4] = d4; data[5] = d5; data[6] = d6; data[7] = d7; } while(0)

namespace RTT
{
    namespace CAN
    {
        /**
         * A dummy CANOpen device with node ID 255.
         */
        struct CANDummyDevice: public CANDeviceInterface
        {
            virtual void process(const CANMessage* msg)
            {
                return;
            }
        };

        typedef ::CpStruct_CAN CANBase;

        inline unsigned int lower_u32(const unsigned char* data)
        {
            return *(unsigned int*) (data);
        }

        inline unsigned int higher_u32(const unsigned char* data)
        {
            return *(unsigned int*) (data + 4);
        }

        /**
         * @brief A CAN message containing message sender and message data.
         */
        struct CANMessage: public CANBase
        {
            /**
             * The type for CAN message ids. It can be standard or extended
             */
            typedef unsigned int ID;
            /**
             * The type for the message flags.
             */
            typedef unsigned int Flags;
            /**
             * The type for the message Data.
             */
            typedef unsigned char Data;
            /**
             * The type for the message timestamp.
             */
            typedef double Seconds;


            /**
             * Create an empty CANMessage.
             * The origin is set to the CANDummyDevice.
             */
            CANMessage() :
                origin(&candevice_dummy)
            {
                clear();
            }

            /**
             * Create an empty CANMessage with a CANOpen device as origin.
             */
            CANMessage(CANDeviceInterface* _origin) :
                origin(_origin)
            {
                clear();
            }

            /**
             * Create a Standard CANMessage with default flags.
             *
             * @param _origin The CANOpen device which created this CANMessage.
             * @param _msgid  The Standard CAN ID of the message.
             * @param _data   A pointer to the data to be used (will be copied).
             * @param _length The length of the data, the number of items in the data array (max 8)
             * @see createStandard for the equivalent factory function.
             */
            CANMessage(CANDeviceInterface* _origin, ID _msgid, Data* _data,
                    unsigned int _length) :
                origin(_origin)
            {
                clear();
                setDLC(_length);
                setStdId(_msgid);
                for (unsigned int i = 0; _data != 0 && i < _length; ++i)
                    setData(i, _data[i]);
            }

            /**
             * Create an Extended CAN Message with default flags.
             * @param _origin The CANOpen device which created this CANMessage.
             * @param _msgid  The Extended CAN ID of the message.
             * @param _data   A pointer to the data to be used (will be copied).
             * @param _length The length of the data, the number of items in the data array (max 8)
             */
            static CANMessage* createExtended(CANDeviceInterface* _origin,
                    ID _msgid, Data* _data, unsigned int _length)
            {
                CANMessage* cm =
                        new CANMessage(_origin, _msgid, _data, _length);
                cm->setExtId(_msgid);
                return cm;
            }

            /**
             * Create a Standard CAN Message with default flags.
             * @param _origin The CANOpen device which created this CANMessage.
             * @param _msgid  The Standard CAN ID of the message.
             * @param _data   A pointer to the data to be used (will be copied).
             * @param _length The length of the data, the number of items in the data array (max 8)
             */
            static CANMessage* createStandard(CANDeviceInterface* _origin,
                    ID _msgid, Data* _data, unsigned int _length)
            {
                return new CANMessage(_origin, _msgid, _data, _length);
            }

            /**
             * Create a Standard CAN Message with the Remote flag set.
             * @param _origin The CANOpen device which created this CANMessage.
             * @param _msgid  The Standard CAN ID of the message.
             * @param _data   A pointer to the data to be used (will be copied).
             * @param _length The length of the data, the number of items in the data array (max 8)
             */
            static CANMessage* createStdRemote(CANDeviceInterface* _origin,
                    ID _msgid, Data* _data, unsigned int _length)
            {
                CANMessage* cm =
                        new CANMessage(_origin, _msgid, _data, _length);
                cm->setRemote();
                return cm;
            }

            /**
             * Create an Extended CAN Message with the Remote flag set.
             * @param _origin The CANOpen device which created this CANMessage.
             * @param _msgid  The Extended CAN ID of the message.
             * @param _data   A pointer to the data to be used (will be copied).
             * @param _length The length of the data, the number of items in the data array (max 8)
             */
            static CANMessage* createExtRemote(CANDeviceInterface* _origin,
                    ID _msgid, Data* _data, unsigned int _length)
            {
                CANMessage* cm =
                        new CANMessage(_origin, _msgid, _data, _length);
                cm->setExtId(_msgid);
                cm->setRemote();
                return cm;
            }

            /**
             * Clear the ID and flags of this CANMessage, except the origin.
             */
            void clear()
            {
                CpMacMsgClear(this);
            }

            /**
             * Check the remote flag.
             */
            bool isRemote() const
            {
                return CpMacIsRemote(this);
            }

            /**
             * Set the remote flag.
             */
            void setRemote()
            {
                CpMacSetRemote(this);
            }

            /**
             * Return data element at position \a pos, starting from 0.
             */
            Data getData(unsigned int pos) const
            {
                return CpMacGetData(this,pos);
            }

            /**
             * Set the data element at position \a pos with value \a d, starting from 0.
             */
            void setData(unsigned int pos, Data d)
            {
                CpMacSetData(this,pos,d);
            }

            /**
             * Set the Data and Data Length Code of this CANMessage.
             */
            void setDataDLC(Data* _data, unsigned int _length)
            {
                CpMacSetDlc(this, _length);
                for (unsigned int i = 0; _data != 0 && i < _length; ++i)
                    setData(i, _data[i]);
            }

            /**
             * Get the Data Length Code of this CANMessage.
             */
            unsigned int getDLC() const
            {
                return CpMacGetDlc(this);
            }

            /**
             * Set the Data Length Code of this CANMessage.
             */
            void setDLC(unsigned int length)
            {
                CpMacSetDlc(this,length);
            }

            bool isExtended() const
            {
                return CpMacIsExtended(this);
            }

            bool isStandard() const
            {
                return !isExtended();
            }

            unsigned int getStdId() const
            {
                return CpMacGetStdId(this);
            }

            unsigned int getExtId() const
            {
                return CpMacGetExtId(this);
            }

            void setStdId(unsigned int id)
            {
                CpMacSetStdId(this,id);
            }

            void setExtId(unsigned int id)
            {
                CpMacSetExtId(this,id);
            }

            /**
             * Compare this CANMessage with another CANMessage.
             * The complete message is compared, except the origin.
             */
            bool operator==(CANMessage& other) const
            {
                if (isStandard() == other.isStandard() && (getStdId()
                        == other.getStdId()) && (getDLC() == other.getDLC()))
                {
                    for (unsigned int i = 0; i < getDLC(); ++i)
                        if (getData(i) != other.getData(i))
                            return false;
                    return true;
                }

                if (isExtended() == other.isExtended() && (getExtId()
                        == other.getExtId()) && (getDLC() == other.getDLC()))
                {
                    for (unsigned int i = 0; i < getDLC(); ++i)
                        if (getData(i) != other.getData(i))
                            return false;
                    return true;
                }
                return false;
            }

            /**
             * Assign a CANMessage \a msg to this CANMessage.
             * The complete message is copied.
             */
            CANMessage& operator=(const CpStruct_CAN &msg)
            {
                v_MsgId = msg.v_MsgId;
                v_MsgFlags = msg.v_MsgFlags;
                memcpy((void*) (v_MsgData), (void*) (msg.v_MsgData), 8);
                return *this;
            }

            /**
             * The sender of this message.
             */
            CANDeviceInterface *origin;
            /**
             * Timestamp of the moment message is received
             */
            Seconds timestamp;
        private:
            /**
             * Used for dummy CANMessage origin.
             */
            static CANDummyDevice candevice_dummy;
        };

    }
}

#endif
