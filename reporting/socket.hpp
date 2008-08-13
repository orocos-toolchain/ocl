/***************************************************************************

                       Socket.h -  Small socket wrapper
                           -------------------
    begin                : Fri Aug 4 2006
    copyright            : (C) 2006 Bas Kemper
    email                : kst@ <my name> .be

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
#ifndef ORO_COMP_SOCKET_H
#define ORO_COMP_SOCKET_H
#include <iostream>

namespace {
    class sockbuf;
};

namespace OCL {
namespace TCP {
    class Socket : public std::ostream {
        friend class ::sockbuf;
        private:
            /**
             * Socket ID
             */
            int socket;

            /**
             * Return true when a line which is already
             * stored in the buffer is available.
             * Terminate the line with \0 and adjust ptrpos to this
             * position.
             *
             * Begin should be the beginning of the new line.
             */
            bool lineAvailable();

            /**
             * Move all data in the buffer to the beginning of the
             * buffer, if needed.
             */
            void checkBufferOverflow();

            /**
             * Close socket without any message to the client.
             */
            void rawClose();

            /**
             * Available data.
             */
            /* buflength should be at least msglength * 2 in order to avoid problems with memcpy! */
            #define BUFLENGTH 2000
            char buffer[BUFLENGTH];
            int begin;
            int ptrpos;
            int end;

        public:
            /**
             * Create an incoming server socket.
             *
             * @param port      Port to listen on.
             */
            Socket( int socketID );
            ~Socket();

            /**
             * Check wether the state of the socket is valid or not.
             */
            bool isValid() const;

            /**
             * Check wether there is new data available.
             */
            bool dataAvailable();

            /**
             * Read a line from the socket.
             */
            std::string readLine();

            /**
             * Close the connection. Send a nice message to the user.
             */
            void close();
    };
};
};
#endif
