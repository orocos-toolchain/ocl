/***************************************************************************

                    command.hpp - Processes client commands
                           -------------------
    begin                : Wed Aug 9 2006
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

#ifndef ORO_COMP_TCP_REPORTING_COMMAND_HPP
#define ORO_COMP_TCP_REPORTING_COMMAND_HPP
#include <vector>
#include <rtt/os/Mutex.hpp>

namespace OCL
{

namespace TCP
{
    class Datasender;
    class Socket;
    class Command;
    class RealCommand;

    /**
     * Reads a line from the client and interprete it.
     */
    class TcpReportingInterpreter
    {
        protected:
            std::vector<Command*> cmds;
            RTT::os::MutexRecursive commands;
            unsigned int parseParameters( std::string& ipt, std::string& cmd, std::string** params );
            Datasender* _parent;

        public:
            /**
             * After setup, the interpreter will only recognize the command
             * 'VERSION 1.0' by default.
             */
            TcpReportingInterpreter(Datasender* parent);
            ~TcpReportingInterpreter();
            void process();

            /**
             * Get the marshaller associated with the current connection.
             */
            Datasender* getConnection() const;

            /**
             * Accept all valid commands (except 'VERSION 1.0')
             */
            void setVersion10();

            /**
             * Return a reference to the command list.
             */
            const std::vector<Command*>& giveCommands() const;

            /**
             * Add support for the given command.
             */
            void addCommand( Command* command );

            /**
             * Remove support for the given command name.
             */
            void removeCommand( const char* name );
    };

    /**
     * Command pattern
     */
    class Command
    {
        protected:
            std::string _name;

        public:
            Command( std::string name );
            virtual ~Command();
            virtual bool is(std::string& cmd) const;

            /**
             * Return a reference to the object which is really responsible
             * for executing this command. This enables multiple names
             * for the same command.
             * Return 0 if no such command is founded.
             */
            virtual RealCommand* getRealCommand(const std::vector<Command*>& cmds) const = 0;

            /**
             * Find the command with the given name in the vector.
             */
            static Command* find(const std::vector<Command*>& cmds, const std::string& cmp);

            /**
             * Compare on name
             */
            bool operator==(const std::string& cmp) const;
            bool operator!=(const std::string& cmp) const;
            bool operator<( const Command& cmp ) const;

            /**
             * Get the name of this command.
             */
            const std::string& getName() const;
    };

    /**
     * Another name for a command
     */
    class AliasCommand : public Command
    {
        private:
            std::string _alias;

        public:
            AliasCommand( std::string name, std::string alias );
            virtual ~AliasCommand() {}
            virtual RealCommand* getRealCommand(const std::vector<Command*>& cmds) const;
    };

    /**
     * Real command which can be executed.
     */
    class RealCommand : public Command
    {
        protected:
            TcpReportingInterpreter* _parent;
            unsigned int _minargs;
            unsigned int _maxargs;
            const char* _syntax;

            /**
             * Main code to be implemented by children.
             */
            virtual void maincode( int argc, std::string* args ) = 0;

            /**
             * Send the correct syntax to the client.
             * Return false.
             */
            bool sendError102() const;

            /**
             * Send the message 101 OK to the client.
             * Return true.
             */
            bool sendOK() const;

            /**
             * Convert the parameter with the given index to upper-case.
             * The caller has to make sure that the given index is a valid index.
             */
            void toupper( std::string* args, int index ) const;

            /**
             * Convert all parameters between the start and stop index to upper-case.
             * The caller has to make sure that both start and stop are valid indexes.
             * stop must be strictly greater than start.
             */
            void toupper( std::string* args, int start, int stop ) const;

            /**
             * Return the socket for this command.
             * Fast shortcut for _parent->getConnection()->getSocket()
             */
            inline Socket& socket() const;

        public:
            RealCommand( std::string name, TcpReportingInterpreter* parent, unsigned int minargs = 0, unsigned int maxargs = 0, const char* syntax = 0);
            virtual ~RealCommand();

            /**
             * Return true if the syntax is correct, false otherwise.
             * Send an error message to the client on incorrect syntax.
             */
            virtual bool correctSyntax( unsigned int argc, std::string* args );

            /**
             * Return syntax information
             */
            const char* getSyntax() const;

            /**
             * Returns this.
             */
            virtual RealCommand* getRealCommand(const std::vector<Command*>& cmds) const;

            /**
             * Execute this command.
             */
            void execute( int argc, std::string* args );
    };
}
}
#endif
