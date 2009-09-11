/***************************************************************************

                      Socket.cpp -  Small socket wrapper
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

#include <cstdio>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <rtt/Logger.hpp>
#include <string.h>
#include "socket.hpp"

using RTT::Logger;

#define MSGLENGTH 100
#if MSGLENGTH * 3 > BUFLENGTH
    #error "MSGLENGTH too long" /* memcpy is used */
#endif

#if __APPLE__
#define SEND_OPTIONS        0
#else
#define SEND_OPTIONS        MSG_NOSIGNAL
#endif

namespace {
    const unsigned int bufsize = 2048;
    class sockbuf : public std::streambuf
    {
        private:
            char* ptr;
            OCL::TCP::Socket* mainClass;

        public:
            sockbuf( OCL::TCP::Socket* m ) : mainClass(m)
            {
                char* ptr = new char[bufsize];
                setp(ptr, ptr + bufsize);   // output buffer
                setg(0, 0, 0);              // input stream: not enabled
#if __APPLE__
                /* Linux uses MSG_NOSIGNAL on the ::send() calls, but Mac OS X
                   supports this as a socket option with SIG_NOSIGPIPE. Just
                   set the socket now with that option and not worry about the
                   MSG_NOSIGNAL's later in each of the send() calls. For 
                   further details, see
http://lists.apple.com/archives/macnetworkprog/2002/Dec/msg00091.html
http://trac.wxwidgets.org/ticket/7150
http://gobby.0x539.de/trac/browser/net6/trunk/src/socket.cpp?rev=224
                */
                int value = 1;
                if (-1 == setsockopt(
                        mainClass->socket, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value)))
                {
                    Logger::log() << Logger::Error << "Error setting socket option. Continuing." << Logger::endl;
                }
 #endif
            }

            ~sockbuf()
            {
                sync();
                delete[] ptr;
            }

            int overflow(int c)
            {
                int ret = 0;
                put_buffer();

                if (c != EOF)
                {
                    if (pbase() == epptr())
                    {
                        put_char(c);
                    } else {
                        ret = sputc(c);
                    }
                } else {
                    ret = EOF;
                }
                return ret;
            }

            int sync()
            {
                put_buffer();
                return 0;
            }

            void put_char(int chr)
            {
                Logger::log() << Logger::Error << "Socket::put_char is unimplemented" << Logger::endl;
            }

            void put_buffer()
            {
                if (pbase() != pptr())
                {
                    int length = (pptr() - pbase());
                    char *buffer = new char[length + 1];

                    strncpy(buffer, pbase(), length);
                    buffer[length] = '\0';

                    // empty strings are not sent
                    if( length && ::send ( mainClass->socket, buffer, length, SEND_OPTIONS ) == -1 )
                    {
                        mainClass->rawClose();
                    }
                    setp(pbase(), epptr());
                    delete[] buffer;
                }
            }
    };
};

namespace OCL {
namespace TCP {
    Socket::Socket( int socketID ) :
            std::ostream( new sockbuf(this) ),
            socket(socketID), begin(0), ptrpos(0), end(0)
    {
    }


    Socket::~Socket()
    {
        if( isValid() )
        {
            rawClose();
        }
    }

    bool Socket::isValid() const
    {
        return socket >= 0;
    }

    bool Socket::dataAvailable()
    {
        return isValid() && lineAvailable();
    }

    bool Socket::lineAvailable()
    {
        int flags = fcntl(socket,F_GETFL);
        fcntl(socket,F_SETFL,flags | O_NONBLOCK);
        int ret =recv(socket,buffer,MSGLENGTH,MSG_PEEK);
        if(ret>0){
            //search for \n or \0
            for(unsigned int i=0;i<MSGLENGTH;++i)
                if( buffer[i] == '\n'){
                    ptrpos=i;
                    return true;
                }
            return false;
        }else if(ret==0){
            rawClose();
        }
        return false;


        /* this if clause allows calling lineAvailable() multiple times
        without reading the actual lines with readLine(). */
        /*
        if( ptrpos < end && buffer[ptrpos] == '\0' ){
            return true;
        }

        while( ptrpos < end ){
            if( buffer[ptrpos] == '\n' ) {
                //   overwrite the \n or \r\n with \0
                if( begin < ptrpos && buffer[ptrpos-1] == '\r' ){
                    buffer[ptrpos-1] = '\0';
                }
                buffer[ptrpos] = '\0';
                return true;
            }
            ++ptrpos;
        }
        return false;
        */
    }

    void Socket::checkBufferOverflow()
    {
        if( end + MSGLENGTH >= BUFLENGTH ) {
            if( ptrpos - begin > MSGLENGTH ) {
                Logger::log() << Logger::Error << "Message length violation" << Logger::endl;
                rawClose();
            } else {
                memcpy( buffer, &buffer[begin], end - begin);
            }
            end -= begin;
            ptrpos -= begin;
            begin = 0;
        }
    }

    std::string Socket::readLine()
    {
        if(dataAvailable()){
            if(0>recv(socket,buffer,sizeof(char[ptrpos+1]),MSG_WAITALL))
                return "";

            return std::string(buffer,ptrpos);
        }
        return "";
        /* ugly C style code to read a line from the socket */


        /*
            while(isValid()){
                // process remaining full lines in the buffer
            if( lineAvailable() ){
                std::string ret(&buffer[begin]);

                if( begin == end - 1 ){
                    // reset to start of buffer when everything is read
                    begin = 0;
                    end = 0;
                    ptrpos = 0;
                } else {
                    ++ptrpos;
                    begin = ptrpos;
                }
                return ret;
            }

            // move data back to the beginning of the buffer (should not occur very often)
            checkBufferOverflow();


            // wait for additional input
            int received = recv(socket, &buffer[end], MSGLENGTH, 0 );
            if( received == 0 || received == -1 ){
                rawClose();
                return "";
            }
            end += received;
        }
        return "";
        */
    }

    void Socket::rawClose()
    {
        if( socket != -1 )
        {
            ::close(socket);
        }
        socket = -1;
        return;
    }

    void Socket::close()
    {
        int _socket = socket;
        socket = -1;

        if( _socket )
        {
            // The user notification is sent non-blocking.
            int flags = fcntl( _socket, F_GETFL, 0 );
            if( flags == -1 )
            {
                flags = 0;
            }
            fcntl( _socket, F_SETFL, flags | O_NONBLOCK );
            ::send ( _socket, "104 Bye bye", 11, SEND_OPTIONS );
            ::close( _socket );
        }
    }
};
};
