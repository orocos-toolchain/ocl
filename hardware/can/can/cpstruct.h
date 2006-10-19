#ifndef ORO_CP_STRUCT_CAN_HPP
#define ORO_CP_STRUCT_CAN_HPP

#include "compiler.h"

/*----------------------------------------------------------------------------*/
/*!
** \def     CP_MSG_TIME
** \brief   CpStruct_CAN configuration
**
** This symbol defines if the element v_MsgTime is included in the structure
** CpStruct_CAN. This structure member is only needed for CAN interfaces that
** can handle time stamps.
**
** \li 0 = CAN message does not store time data
** \li 1 = CAN message can store time data
**
*/
#define  CP_MSG_TIME       0


/*----------------------------------------------------------------------------*/
/*!
** \def     CP_USR_DATA
** \brief   CpStruct_CAN configuration
**
** This symbol defines if the element v_UsrData is included in the structure
** CpStruct_CAN. This structure member is only needed for CAN interfaces if
** the application needs to store a 32-bit value for each message.
**
** \li 0 = CAN message does not store user data
** \li 1 = CAN message can store user data
**
*/
#define  CP_USR_DATA       0

/*----------------------------------------------------------------------------*/
/*!
** \struct  CpStruct_CAN   cpconst.h
** \brief   CAN message structure
**
** For transmission and reception of CAN messages a structure which holds
** all necessary informations is used. The structure has the following
** data fields:
*/
typedef struct {
   /*!   The identifier field may have 11 bits for standard frames
   **    (CAN specification 2.0A) or 29 bits for extended frames
   **    (CAN specification 2.0B). The three most significant bits
   **    are reserved for special functionality (the LSB is Bit 0,
   **    the MSB is Bit 31 ).<p>
   **    <ul>
   **    <li>Bit 31: Bit value 1 marks the identifier as an
   **                extended frame. Bit value 0 marks the
   **                identifier as a standard frame.
   **    <li>Bit 30: Bit value 1 marks the identifier
   **                as an remote transmission (RTR).
   **    <li>Bit 29: Reserved for future use
   **    </ul>
   */
   _U32  v_MsgId;

   /*!   The message flags field contains the data length code
   **    (DLC) of the CAN message and the buffer number when
   **    using a FullCAN controller.<p>
   **    The data length code (<b>Bit 0 - Bit 3</b>) contains the
   **    number of data bytes which are transmitted by a message.
   **    The possible value range for the data length code is
   **    from 0 to 8 (bytes).<p>
   **    A FullCAN controller (e.g. AN82527) has more than only
   **    one transmit and one receive buffer and offers more
   **    sophisticated message filtering. The field message
   **    buffer (<b>Bit 4 - Bit 7</b>) specifies the buffer for
   **    message transmission or reception.<p>
   **    The high word (<b>Bit 16 - Bit 31</b>)
   **    is reserved for user defined data.
   */
   _U32  v_MsgFlags;

   /*!   The data fields contain up to eight bytes for a CAN
   **    message. If the data length code is less than 8, the
   **    value of the unused data bytes will be undefined.
   */
   _U08  v_MsgData[8];

#if CP_MSG_TIME == 1
   /*!   The time stamp field defines the time when a CAN message
   **    was received by the CAN controller. The time stamp is a
   **    relative value, which is created by a free running timer.
   **    The time base is one microsecond (1 us). This means a
   **    maximum time span of 4294,96 seconds (1 hour 11 minutes)
   **    between two messages can be measured. This is an optional
   **    field (available if #CP_MSG_TIME is set to 1).
   */
   _U32  v_MsgTime;
#endif

#if CP_USR_DATA == 1
   /*!   The field user data can hold a 32 bit value, which is
   **    defined by the user. This is an optional field
   **    (available if #CP_USR_DATA is set to 1).
   */
   _U32  v_UsrData;
#endif

} CpStruct_CAN;

#endif
