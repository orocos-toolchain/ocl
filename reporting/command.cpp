/***************************************************************************

        command.cpp - Processes client commands
           -------------------
    begin        : Wed Aug 9 2006
    copyright    : (C) 2006 Bas Kemper
    email        : kst@ <my name> .be

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

#include <algorithm> /* std::transform is needed for upper-case conversion */
#include <string>
#include <cstdlib> /* strtoull */
#include <cctype>
#include <cerrno>
#include <rtt/Property.hpp>
#include "NiceHeaderMarshaller.hpp"
#include "TcpReporting.hpp"
#include "command.hpp"
#include "socket.hpp"
#include "datasender.hpp"
#include "socketmarshaller.hpp"

using OCL::TCP::RealCommand;
using OCL::TCP::Socket;
using OCL::TCP::TcpReportingInterpreter;

namespace
{
        /**
         * base::Buffer to prefix each line with '300 '
         */
        class prefixbuf : public std::streambuf
        {
            private:
                std::streambuf* _opt;
                bool newline;

            public:
                prefixbuf( std::streambuf* opt ) : _opt(opt)
                {
                    setp(0, 0);
                    setg(0, 0, 0);
                    newline = true;
                }

                ~prefixbuf()
                {
                    sync();
                }

            protected:
                int overflow(int c)
                {
                    if( c != EOF )
                    {
                        if( newline )
                        {
                            if( _opt->sputn("300 ", 4) != 4 )
                            {
                                return EOF;
                            }
                            newline = false;
                        }
                        int ret = _opt->sputc(c);
                        newline = ( c == '\n' );
                        return ret;
                    }
                    return 0;
                }

                int sync()
                {
                    return _opt->pubsync();
                }
        };

        /**
         * Prefix output stream.
         */
        class prefixstr : public std::ostream
        {
            public:
                prefixstr(std::ostream& opt)
                : std::ostream( new prefixbuf( opt.rdbuf() ) )
                {
                }

                ~prefixstr()
                {
                    delete rdbuf();
                }
        };

    /* std::toupper is implemented as a macro and cannot be used by
    std::transform without a wrapper function. */
    char to_upper (const char c)
    {
        return toupper(c);
    }

    /**
     * Output a XML file (prefixed with '300 ') containing the header to the socket.
     */
    class HeaderCommand : public RealCommand
    {
        protected:
            void maincode( int, std::string* )
            {
                std::vector<std::string> list = _parent->getConnection()->getMarshaller()->getReporter()->getReport()->list();
                for(unsigned int i=0;i<list.size();i++)
                    socket()<<"305 "<<list[i]<<std::endl;
                socket() << "306 End of list" << std::endl;
            }

        public:
            HeaderCommand(TcpReportingInterpreter* parent)
            : RealCommand( "HEADERS", parent )
            {
            }

            ~HeaderCommand()
            {}

            void manualExecute()
            {
                maincode(0,0);
            }
    };

    class HelpCommand : public RealCommand
    {
        protected:
            /**
             * Print list of available commands to the socket.
             */
            void printCommands()
            {
                const std::vector<Command*> &cmds = _parent->giveCommands();
                socket() << "Use HELP <command>" << std::endl;
                for( unsigned int i = 0; i < cmds.size(); i++ )
                {
                    if( cmds[i] == cmds[i]->getRealCommand( cmds ) )
                    {
                        socket() << cmds[i]->getName() << '\n';
                    }
                }
                socket() << '.' << std::endl;
            }

            /**
             * Print help for <command> with <name> to the socket.
             */
            void printHelpFor( const std::string& name, const RealCommand* command )
            {
                socket() << "Name: " << name << std::endl;
                socket() << "Usage: " << name;
                if( command->getSyntax() )
                {
                    socket() << " " << command->getSyntax();
                }
                socket() << std::endl;
            }

            /**
             * Print help for the given command to the socket.
             */
            void printHelpFor( const std::string& cmd )
            {
                const std::vector<Command*> &cmds = _parent->giveCommands();
                for( unsigned int i = 0; i < cmds.size(); i++ )
                {
                    if( cmds[i]->getName() == cmd )
                    {
                        printHelpFor( cmd, cmds[i]->getRealCommand( cmds ) );
                        return;
                    }
                }
                printCommands();
            }

            void maincode( int argc, std::string* params )
            {
                if( argc == 0 )
                {
                    printCommands();
                } else {
                    printHelpFor(params[0]);
                }
            }
        public:
            HelpCommand(TcpReportingInterpreter* parent)
            : RealCommand( "HELP", parent, 0, 1, "[nothing | <command name>]" )
            {
            }
    };

    class ListCommand : public RealCommand
    {
        protected:
            void maincode( int, std::string* )
            {
                socket() << "103 none" << std::endl;
            }

        public:
            ListCommand(TcpReportingInterpreter* parent)
                : RealCommand( "LISTEXTENSIONS", parent )
            {
            }
    };

    class QuitCommand : public RealCommand
    {
        protected:
            void maincode( int, std::string* )
            {
              // The main marshaller is not notified about the connection
              // being closed but will detect it and delete the DataSender
              // in the next serialize() run because DataSender is a
              // thread and this method is (indirectly) called by
              // DataSender::loop.
              socket().close();
            }

        public:
            QuitCommand(TcpReportingInterpreter* parent)
                : RealCommand( "QUIT", parent )
            {
            }
    };

    class SetLimitCommand : public RealCommand
    {
        protected:
            void maincode( int, std::string* args )
            {
                int olderr = errno;
                char* tailptr;
                unsigned long long limit = strtoull( args[0].c_str(), &tailptr, 10 );
                if( *tailptr != '\0' || ( errno != olderr && errno == ERANGE ) )
                {
                    sendError102();
                } else {
                    _parent->getConnection()->setLimit(limit);
                    sendOK();
                }
            }

        public:
            SetLimitCommand(TcpReportingInterpreter* parent)
            : RealCommand( "SETLIMIT", parent, 1, 1, "<frameid>" )
            {
            }
    };

    /**
     * Disable/enable output of data on the socket.
     */
    class SilenceCommand : public RealCommand
    {
        protected:
            void maincode( int, std::string* args )
            {
                toupper( args, 0 );
                if( args[0] == "ON" )
                {
                    _parent->getConnection()->silence(true);
                } else if( args[0] == "OFF") {
                    _parent->getConnection()->silence(false);
                } else {
                    sendError102();
                    return;
                }
                socket() << "107 Silence " << args[0] << std::endl;
            }

        public:
            SilenceCommand(TcpReportingInterpreter* parent)
            : RealCommand( "SILENCE", parent, 1, 1, "[ON | OFF]" )
            {
            }
    };

    /**
     * Add data stream to be printed
     */
    class SubscribeCommand : public RealCommand
    {
        // TODO: id's elimneren voor subscribe command
        // TODO: id's elimneren voor unsubscribe command
        // TODO: id's elimneren
        protected:
            void maincode( int, std::string* args )
            {
                if( _parent->getConnection()->addSubscription(args[0]) )
                {
                    socket() << "302 " << args[0] << std::endl;
                } else {
                    socket() << "301 " << args[0] << std::endl;
                }
            }

        public:
            SubscribeCommand(TcpReportingInterpreter* parent)
            : RealCommand( "SUBSCRIBE", parent, 1, 1, "<source name>" )
            {
            }
    };

    class SubscriptionsCommand : public RealCommand
    {
        protected:
            void maincode( int, std::string* )
            {
                _parent->getConnection()->listSubscriptions();
            }

        public:
            SubscriptionsCommand(TcpReportingInterpreter* parent)
            : RealCommand( "SUBS", parent )
            {
            }
    };

    class UnsubscribeCommand : public RealCommand
    {
        protected:
            void maincode( int, std::string* args )
            {
                if( _parent->getConnection()->removeSubscription(args[0]) )
                {
                    socket() << "303 " << args[0] << std::endl;
                } else {
                    socket() << "304 " << args[0] << std::endl;
                }
            }

        public:
            UnsubscribeCommand(TcpReportingInterpreter* parent)
                : RealCommand( "UNSUBSCRIBE", parent, 1, 1, "<source name>" )
                {
                }
    };

    class VersionCommand : public RealCommand
    {
        protected:
            void maincode( int, std::string* args )
            {
                if( args[0] == "1.0" )
                {
                    _parent->setVersion10();
                    sendOK();
                } else {
                    socket() << "106 Not supported" << std::endl;
                }
            }

        public:
            VersionCommand(TcpReportingInterpreter* parent)
            : RealCommand( "VERSION", parent, 1, 1, "1.0" )
            {
            }
    };
}

namespace OCL
{
namespace TCP
{
    /*
     * The default Orocos Command objects are not used since we
     * do not use Data Sources here.
     */
    class RealCommand;

    Command::Command( std::string name )
        : _name( name )
    {
    }

    Command::~Command()
    {
    }

    Command* Command::find(const std::vector<Command*>& cmds, const std::string& cmp)
    {
        for( unsigned int j = 0; j < cmds.size(); j++ )
        {
            if( *cmds[j] == cmp )
            {
                return cmds[j];
            }
        }
        return 0;
    }

    const std::string& Command::getName() const
    {
        return _name;
    }

    bool Command::is(std::string& cmd) const
    {
        return cmd == _name;
    }

    bool Command::operator==(const std::string& cmp) const
    {
        return cmp == _name;
    }

    bool Command::operator!=(const std::string& cmp) const
    {
        return cmp != _name;
    }

    bool Command::operator<( const Command& cmp ) const
    {
        return _name < cmp.getName();
    }

    AliasCommand::AliasCommand( std::string name, std::string alias )
    : Command( name ), _alias( alias )
    {
    }

    RealCommand* AliasCommand::getRealCommand(const std::vector<Command*>& cmds) const
    {
        Command* ret = Command::find( cmds, _alias );
        if( !ret )
        {
            return 0;
        }
        return ret->getRealCommand(cmds);
    }

    RealCommand::RealCommand( std::string name, TcpReportingInterpreter* parent, unsigned int minargs,
    unsigned int maxargs, const char* syntax )
    : Command( name ), _parent( parent ), _minargs( minargs ), _maxargs( maxargs ), _syntax( syntax )
    {
    }

    RealCommand::~RealCommand()
    {
    }

    const char* RealCommand::getSyntax() const
    {
        return _syntax;
    }

    bool RealCommand::sendError102() const
    {
        if( _syntax )
        {
            socket() << "102 Syntax: " << _name << ' ' << _syntax << std::endl;
        } else {
            socket() << "102 Syntax: " << _name << std::endl;
        }
        return false;
    }

    bool RealCommand::sendOK() const
    {
        socket() << "101 OK" << std::endl;
        return true;
    }

    bool RealCommand::correctSyntax( unsigned int argc, std::string* )
    {
        if( argc < _minargs || argc > _maxargs )
        {
            return sendError102();
        }
        return true;
    }

    RealCommand* RealCommand::getRealCommand(const std::vector<Command*>& cmds) const
    {
        return const_cast<RealCommand*>(this);
    }

    void RealCommand::execute( int argc, std::string* args )
    {
        if( correctSyntax( argc, args ) )
        {
            maincode( argc, args );
        }
    }

    void RealCommand::toupper( std::string* args, int i ) const
    {
        std::transform( args[i].begin(), args[i].end(), args[i].begin(), to_upper );
    }

    void RealCommand::toupper( std::string* args, int start, int stop ) const
    {
        for( int i = start; i <= stop; i++ )
        {
            toupper( args, i );
        }
    }

    Socket& RealCommand::socket() const
    {
        return _parent->getConnection()->getSocket();
    }

    TcpReportingInterpreter::TcpReportingInterpreter(Datasender* parent)
        : _parent( parent )
    {
        addCommand( new VersionCommand(this) );
        addCommand( new HelpCommand(this) );
        addCommand( new QuitCommand(this) );
        addCommand( new AliasCommand( "EXIT", "QUIT" ) );
    }

    TcpReportingInterpreter::~TcpReportingInterpreter()
    {
        for( std::vector<Command*>::iterator i = cmds.begin();
         i != cmds.end();
         i++ )
        {
            delete *i;
        }
    }

    void TcpReportingInterpreter::addCommand( Command* command )
    {
        // this method has a complexity of O(n) because we want
        // the list to be sorted.
        commands.lock();
        std::vector<Command*>::iterator i = cmds.begin();
        while( i != cmds.end() && *command < **i ) {
            i++;
        }

        // avoid duplicates
        if( i != cmds.end() && *command == (*i)->getName() )
        {
            return;
        }
        cmds.insert( i, command );
        commands.unlock();
    }

    const std::vector<Command*>& TcpReportingInterpreter::giveCommands() const
    {
        return cmds;
    }

    Datasender* TcpReportingInterpreter::getConnection() const
    {
        return _parent;
    }

    void TcpReportingInterpreter::process()
    {
        std::string ipt = getConnection()->getSocket().readLine();

        if( ipt.empty() )
        {
            return;
        }

        /* parseParameters returns data by reference */
        std::string cmd;
        std::string* params;

        unsigned int argc = parseParameters( ipt, cmd, &params );

        std::transform( cmd.begin(), cmd.end(), cmd.begin(), to_upper );

        /* search the command to be executed */
        bool correct = false;
        commands.lock();
        Command* obj = Command::find( cmds, cmd );
        if( obj ) /* command found */
        {
            RealCommand* rcommand = obj->getRealCommand(cmds);
            if( rcommand ) /* alias is correct */
            {
                rcommand->execute( argc, params );
                correct = true;
            }
        } else {
            Logger::log() << Logger::Error << "Invalid command: " << ipt << Logger::endl;
        }
        commands.unlock();

        if( !correct )
        {
            getConnection()->getSocket() << "105 Command not found" << std::endl;
        }
    }

    unsigned int TcpReportingInterpreter::parseParameters(
        std::string& ipt, std::string& cmd, std::string** params )
    {
        unsigned int argc = 0;
        std::string::size_type pos = ipt.find_first_of("\t ", 0);
        while( pos != std::string::npos )
        {
            pos = ipt.find_first_of("\t ", pos + 1);
            argc++;
        }
        if( argc > 0 )
        {
            *params = new std::string[argc];
            pos = ipt.find_first_of("\t ", 0);
            cmd = ipt.substr(0, pos);
            unsigned int npos;
            for( unsigned int i = 0; i < argc; i++ )
            {
                npos = ipt.find_first_of("\t ", pos + 1);
                (*params)[i] = ipt.substr(pos+1,npos - pos - 1);
                pos = npos;
            }
        } else {
            cmd = ipt;
            *params = 0;
        }
        return argc;
    }

    void TcpReportingInterpreter::removeCommand( const char* ipt )
    {
        commands.lock();
        std::vector<Command*>::iterator i = cmds.begin();
        while( i != cmds.end() && **i != ipt ) {
            i++;
        }
        if( i == cmds.end() )
        {
            Logger::log() << Logger::Error << "TcpReportingInterpreter::removeCommand: removing unknown command" << ipt << Logger::endl;
        } else {
            Command* todel = *i;
            cmds.erase(i);
            delete todel;
        }
        commands.unlock();
    }

    void TcpReportingInterpreter::setVersion10()
    {
        commands.lock();
        removeCommand( "VERSION" );
        addCommand( new ListCommand(this) );
        addCommand( new HeaderCommand(this) );
        addCommand( new SilenceCommand(this) );
        addCommand( new SetLimitCommand(this) );
        addCommand( new SubscribeCommand(this) );
        addCommand( new UnsubscribeCommand(this) );
        addCommand( new SubscriptionsCommand(this) );
        commands.unlock();
        _parent->silence( false );
    }
}
}

