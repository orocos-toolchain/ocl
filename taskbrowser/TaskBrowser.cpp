#ifndef NO_GPL
/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:31:34 CEST 2008  TaskBrowser.cpp

                        TaskBrowser.cpp -  description
                           -------------------
    begin                : Thu July 03 2008
    copyright            : (C) 2008 Peter Soetens
    email                : peter.soetens@fmtc.be

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this program; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 ***************************************************************************/
#else
/***************************************************************************
  tag: Peter Soetens  Tue Dec 21 22:43:07 CET 2004  TaskBrowser.cxx

                        TaskBrowser.cxx -  description
                           -------------------
    begin                : Tue December 21 2004
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
#endif


#include <rtt/Logger.hpp>
#include <rtt/extras/MultiVector.hpp>
#include <rtt/types/TypeStream.hpp>
#include <rtt/types/Types.hpp>
#include "TaskBrowser.hpp"

#include <rtt/scripting/TryCommand.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/plugin/PluginLoader.hpp>
#include <rtt/scripting/parser-debug.hpp>
#include <rtt/scripting/Parser.hpp>
#include <rtt/scripting/parse_exception.hpp>
#include <rtt/scripting/PeerParser.hpp>
#include <rtt/scripting/Scripting.hpp>
#include <rtt/plugin/PluginLoader.hpp>
#include <rtt/internal/GlobalService.hpp>
#include <rtt/types/GlobalsRepository.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <deque>
#include <stdio.h>
#include <algorithm>

#if defined(HAS_READLINE) && !defined(NO_GPL)
# define USE_READLINE
#endif
#if defined(HAS_EDITLINE)
// we use the readline bc wrapper:
# define USE_READLINE
# define USE_EDITLINE
#endif
// only use signals if posix, and pure readline
# if defined(_POSIX_VERSION) && defined(HAS_READLINE) && !defined(HAS_EDITLINE)
#   define USE_SIGNALS 1
# endif


#ifdef USE_READLINE
# ifdef USE_EDITLINE
#  include <editline/readline.h>
#  include <editline/history.h>
# else
#  include <readline/readline.h>
#  include <readline/history.h>
# endif
#endif
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

#ifdef USE_SIGNALS
#include <signal.h>
#endif

// we need to declare it since Xenomai does not declare it in any header
#if defined(USE_SIGNALS) && defined(OROCOS_TARGET_XENOMAI) && CONFIG_XENO_VERSION_MAJOR == 2 && CONFIG_XENO_VERSION_MINOR >= 5
extern "C"
int xeno_sigwinch_handler(int sig, siginfo_t *si, void *ctxt);
#endif
namespace OCL
{
    using namespace boost;
    using namespace std;
    using namespace RTT;
    using namespace RTT::detail;
#ifdef USE_READLINE
    std::vector<std::string> TaskBrowser::candidates;
    std::vector<std::string> TaskBrowser::completes;
    std::vector<std::string>::iterator TaskBrowser::complete_iter;
    std::string TaskBrowser::component;
    std::string TaskBrowser::component_found;
    std::string TaskBrowser::peerpath;
    std::string TaskBrowser::text;
#endif
    RTT::TaskContext* TaskBrowser::taskcontext = 0;
    Service::shared_ptr TaskBrowser::taskobject;
    RTT::TaskContext* TaskBrowser::peer = 0;
    RTT::TaskContext* TaskBrowser::tb = 0;
    RTT::TaskContext* TaskBrowser::context = 0;

    using boost::bind;
    using namespace RTT;
    using namespace std;

    string TaskBrowser::red;
    string TaskBrowser::green;
    string TaskBrowser::blue;
    std::deque<TaskContext*> taskHistory;
    std::string TaskBrowser::prompt("> ");
    std::string TaskBrowser::coloron;
    std::string TaskBrowser::underline;
    std::string TaskBrowser::coloroff;


    /**
     * Our own defined "\n"
     */
    static std::ostream&
    nl(std::ostream& __os)
    { return __os.put(__os.widen('\n')); }

    // All readline specific functions
#if defined(USE_READLINE)

    // Signal code only on Posix:
#if defined(USE_SIGNALS)

    void TaskBrowser::rl_sigwinch_handler(int sig, siginfo_t *si, void *ctxt) {
#if defined(OROCOS_TARGET_XENOMAI) && CONFIG_XENO_VERSION_MAJOR == 2 && CONFIG_XENO_VERSION_MINOR >= 5
        if (xeno_sigwinch_handler(sig, si, ctxt) == 0)
#endif
            rl_resize_terminal();
    }
#endif // USE_SIGNALS

    char *TaskBrowser::rl_gets ()
    {
        /* If the buffer has already been allocated,
           return the memory to the free pool. */
        if (line_read)
            {
#ifdef _WIN32
	      /**
	       * If you don't have this function, download the readline5.2 port
	       * from :
	       http://gpsim.sourceforge.net/gpsimWin32/packages/readline-5.2-20061112-src.zip
	       http://gpsim.sourceforge.net/gpsimWin32/packages/readline-5.2-20061112-lib.zip
	       http://gpsim.sourceforge.net/gpsimWin32/packages/readline-5.2-20061112-bin.zip
	       *
	       * free(line_read) will cause crashes since the malloc has been
	       * done in another DLL. You can emulate rl_free by adding this function
	       * yourself to your readline library and let it free its argument.
	       */
	      free(line_read);
#else
                free (line_read);
#endif
                line_read = 0;
            }

        /* Get a line from the user. */
        std::string p;
        if ( !macrorecording ) {
            p = prompt;
        } else {
            p = "> ";
        }
#if defined(USE_SIGNALS)

        if (rl_set_signals() != 0)
            cerr << "Error setting signals !" <<endl;
#endif
        line_read = readline ( p.c_str() );

        /* If the line has any text in it,
           save it on the history. */
        if (line_read && *line_read) {
            // do not store "quit"
            string s = line_read;
            if (s != "quit" && ! ( history_get( where_history() ) && s == string(history_get( where_history() )->line) ) ) {
//                cout << "Where: " << where_history() << " history_get: " << ( history_get( where_history() ) ? history_get( where_history() )->line : "(null)") << endl;
//                cout << "History: " << (current_history()  ? (const char*) current_history()->line : "(null)") << endl;
                add_history (line_read);
            }
        }
        return (line_read);
    }

    char* TaskBrowser::dupstr( const char *s )
    {
        char * rv;
        // must be C-style :
        rv = (char*) malloc( strlen( s ) + 1 );
        strncpy( rv, s, strlen(s) + 1 );
        return rv;
    }

    char *TaskBrowser::command_generator( const char *_text, int state )
    {
        // first time called.
        if ( !state )
            {
                // make a copy :
                text = _text;
                // rebuild a completion list
                completes.clear();
                find_completes();
                complete_iter = completes.begin();
            }
        else
            ++complete_iter;

        // return zero if nothing more is found
        if ( complete_iter == completes.end() )
            return 0;
        // return the next completion option
        return  dupstr( complete_iter->c_str() ); // RL requires malloc !
    }

    /**
     * This is the entry function to look up all possible completions at once.
     *
     */
    void TaskBrowser::find_completes() {
        std::string::size_type pos;
        std::string::size_type startpos;
        std::string line( rl_line_buffer, rl_point );

        // complete on 'cd' or 'ls' :
        if ( line.find(std::string("cd ")) == 0 || line.find(std::string("ls ")) == 0) {
            //cerr <<endl<< "switch to :" << text<<endl;
//             pos = text.rfind(".");
            pos = line.find(" ");      // pos+1 is first peername
            startpos = line.find_last_of(". "); // find last peer
            //cerr <<"startpos :"<<startpos<<endl;
            // start searching from component.
            peer = taskcontext;
            if ( pos+1 != line.length() ) // bounds check
                peer = findPeer( line.substr(pos+1) );

            if (!peer)
                return;
            //std::string peername = text.substring( pos+1, std::string::npos );
            RTT::TaskContext::PeerList v = peer->getPeerList();
            for (RTT::TaskContext::PeerList::iterator i = v.begin(); i != v.end(); ++i) {
                std::string path;
                if ( !( pos+1 > startpos) )
                    path = line.substr(pos+1, startpos - pos);
                //cerr << "path :"<<path<<endl;
                if ( *i == line.substr(startpos+1) )
                     completes.push_back( path + *i + ".");
                else
                    if ( startpos == std::string::npos || startpos+1 == line.length() || i->find( line.substr(startpos+1)) == 0 )
                        completes.push_back( path + *i );
            }
            // Stop here if 'cd'
            if (line.find(std::string("cd ")) == 0)
                return;
            // Add objects for 'ls'.
            v = peer->provides()->getProviderNames();
            for (RTT::TaskContext::PeerList::iterator i = v.begin(); i != v.end(); ++i) {
                std::string path;
                if ( !( pos+1 > startpos) )
                    path = line.substr(pos+1, startpos - pos);
                //cerr << "path :"<<path<<endl;
                if ( *i == line.substr(startpos+1) )
                     completes.push_back( path + *i + ".");
                else
                    if ( startpos == std::string::npos || startpos+1 == line.length() || i->find( line.substr(startpos+1)) == 0 )
                        completes.push_back( path + *i );
            }
            return; // do not add component names.
        }

        // TaskBrowser commands :
        if ( line.find(std::string(".")) == 0 ) {
            // first make a list of all sensible completions.
            std::vector<std::string> tbcoms;
            tbcoms.push_back(".loadProgram ");
            tbcoms.push_back(".unloadProgram ");
            tbcoms.push_back(".loadStateMachine ");
            tbcoms.push_back(".unloadStateMachine ");
            tbcoms.push_back(".light");
            tbcoms.push_back(".dark");
            tbcoms.push_back(".hex");
            tbcoms.push_back(".nohex");
            tbcoms.push_back(".nocolors");
            tbcoms.push_back(".connect");
            tbcoms.push_back(".record");
            tbcoms.push_back(".end");
            tbcoms.push_back(".cancel");
            tbcoms.push_back(".provide");
            tbcoms.push_back(".services");
            tbcoms.push_back(".typekits");
            tbcoms.push_back(".types");

            // then see which one matches the already typed line :
            for( std::vector<std::string>::iterator it = tbcoms.begin();
                 it != tbcoms.end();
                 ++it)
                if ( it->find(line) == 0 )
                    completes.push_back( *it ); // if partial match, add.
            return;
        }

        if ( line.find(std::string("list ")) == 0
             || line.find(std::string("trace ")) == 0
             || line.find(std::string("untrace ")) == 0) {
            stringstream ss( line.c_str() ); // copy line into ss.
            string lcommand;
            ss >> lcommand;
            lcommand += ' ';
            std::vector<std::string> progs;

            if ( context->provides()->hasService("scripting") ) {
                // THIS:
                progs = context->getProvider<Scripting>("scripting")->getProgramList();
                // then see which one matches the already typed line :
                for( std::vector<std::string>::iterator it = progs.begin();
                        it != progs.end();
                        ++it) {
                    string res = lcommand + *it;
                    if ( res.find(line) == 0 )
                        completes.push_back( *it ); // if partial match, add.
                }
                progs = context->getProvider<Scripting>("scripting")->getStateMachineList();
                for( std::vector<std::string>::iterator it = progs.begin();
                        it != progs.end();
                        ++it) {
                    string res = lcommand + *it;
                    if ( res.find(line) == 0 )
                        completes.push_back( *it ); // if partial match, add.
                }
            }
            return;
        }

        startpos = text.find_last_of(",( ");
        if ( startpos == std::string::npos )
            startpos = 0;      // if none is found, start at beginning

        // complete on peers and objects, and find the peer the user wants completion for
        find_peers(startpos);
        // now proceed with 'this->peer' as TC,
        // this->taskobject as TO and
        // this->component as object and
        // this->peerpath as the text leading up to 'this->component'.

        // store the partial compname or peername
        std::string comp = component;


        // NEXT: use taskobject to complete commands, events, methods, attrs
        // based on 'component' (find trick).
        // if taskobject == peer, also complete properties
        find_ops();

        // TODO: concat two cases below as text.find("cd")...
        // check if the user is tabbing on an empty command, then add the console commands :
        if (  line.empty() ) {
            completes.push_back("cd ");
            completes.push_back("cd ..");
            completes.push_back("ls");
            completes.push_back("help");
            completes.push_back("quit");
            completes.push_back("list");
            completes.push_back("trace");
            completes.push_back("untrace");
            if (taskcontext == context)
                completes.push_back("leave");
            else
                completes.push_back("enter");
            // go on below to add components and tasks as well.
        }

        // only try this if text is not empty.
        if ( !text.empty() ) {
            if ( std::string( "cd " ).find(text) == 0 )
                completes.push_back("cd ");
            if ( std::string( "ls" ).find(text) == 0 )
                completes.push_back("ls");
            if ( std::string( "cd .." ).find(text) == 0 )
                completes.push_back("cd ..");
            if ( std::string( "help" ).find(text) == 0 )
                completes.push_back("help");
            if ( std::string( "quit" ).find(text) == 0 )
                completes.push_back("quit");
            if ( std::string( "list " ).find(text) == 0 )
                completes.push_back("list ");
            if ( std::string( "trace " ).find(text) == 0 )
                completes.push_back("trace ");
            if ( std::string( "untrace " ).find(text) == 0 )
                completes.push_back("untrace ");

            if (taskcontext == context && string("leave").find(text) == 0)
                completes.push_back("leave");

            if (context == tb && string("enter").find(text) == 0)
                completes.push_back("enter");
        }
    }

    void TaskBrowser::find_ops()
    {
        // the last (incomplete) text is stored in 'component'.
        // all attributes :
        std::vector<std::string> attrs;
        attrs = taskobject->getAttributeNames();
        for (std::vector<std::string>::iterator i = attrs.begin(); i!= attrs.end(); ++i ) {
            if ( i->find( component ) == 0 )
                completes.push_back( peerpath + *i );
        }
        // all properties if RTT::TaskContext/Service:
        std::vector<std::string> props;
        taskobject->properties()->list(props);
        for (std::vector<std::string>::iterator i = props.begin(); i!= props.end(); ++i ) {
            if ( i->find( component ) == 0 ) {
                completes.push_back( peerpath + *i );
            }
        }

        // methods:
        vector<string> comps = taskobject->getNames();
        for (std::vector<std::string>::iterator i = comps.begin(); i!= comps.end(); ++i ) {
            if ( i->find( component ) == 0  )
                completes.push_back( peerpath + *i );
        }

        // types:
        comps = Types()->getDottedTypes();
        for (std::vector<std::string>::iterator i = comps.begin(); i!= comps.end(); ++i ) {
            if ( peerpath.empty() && i->find( component ) == 0  )
                completes.push_back( *i );
        }

        // Global Attributes:
        comps = GlobalsRepository::Instance()->getAttributeNames();
        for (std::vector<std::string>::iterator i = comps.begin(); i!= comps.end(); ++i ) {
            if ( peerpath.empty() && i->find( component ) == 0  )
                completes.push_back( *i );
        }

        // Global methods:
        if ( taskobject == peer->provides() && peer == context) {
            comps = GlobalService::Instance()->getNames();
            for (std::vector<std::string>::iterator i = comps.begin(); i!= comps.end(); ++i ) {
                if ( i->find( component ) == 0  )
                    completes.push_back( peerpath + *i );
            }
        }

        // complete on types:
        bool try_deeper = false;
        try {
            Parser parser;
            DataSourceBase::shared_ptr result = parser.parseExpression( peerpath + component_found, context );
            if (result && !component.empty() ) {
                vector<string> members = result->getMemberNames();
                for (std::vector<std::string>::iterator i = members.begin(); i!= members.end(); ++i ) {
                    if ( string( component_found + "." + *i ).find( component ) == 0  )
                        completes.push_back( peerpath + component_found + "." + *i );
                    if ( component_found + "." + *i == component )
                        try_deeper = true;
                }
            }
        } catch(...) {}
        // this is a hack to initiate a complete on a valid expression that might have members.
        // the completer above would only return the expression itself, while this one tries to 
        // go a level deeper again.
        if (try_deeper) {
            try {
                Parser parser;
                DataSourceBase::shared_ptr result = parser.parseExpression( peerpath + component, context );
                if (result && !component.empty() ) {
                    vector<string> members = result->getMemberNames();
                    for (std::vector<std::string>::iterator i = members.begin(); i!= members.end(); ++i ) {
                        if (component_found + "." != component ) // catch corner case.
                            completes.push_back( peerpath + component + "." + *i );
                    }
                }
            } catch(...) {}
        }
    }

    void TaskBrowser::find_peers( std::string::size_type startpos )
    {
        peerpath.clear();
        peer = context;
        taskobject = context->provides();

        std::string to_parse = text.substr(startpos);
        startpos = 0;
        std::string::size_type endpos = 0;
        // Traverse the entered peer-list
        component.clear();
        peerpath.clear();
        // This loop separates the peer/service from the member/method
        while (endpos != std::string::npos )
            {
                bool itemfound = false;
                endpos = to_parse.find(".");
                if ( endpos == startpos ) {
                    component.clear();
                    break;
                }
                std::string item = to_parse.substr(startpos, endpos);

                if ( taskobject->hasService( item ) ) {
                    taskobject = peer->provides(item);
                    itemfound = true;
                } else
                    if ( peer->hasPeer( item ) ) {
                        peer = peer->getPeer( item );
                        taskobject = peer->provides();
                        itemfound = true;
                    } else if ( GlobalService::Instance()->hasService(item) ) {
                        taskobject = GlobalService::Instance()->provides(item);
                        itemfound = true;
                    }
                if ( itemfound ) { // if "." found and correct path
                    peerpath += to_parse.substr(startpos, endpos) + ".";
                    if ( endpos != std::string::npos )
                        to_parse = to_parse.substr(endpos + 1);
                    else
                        to_parse.clear();
                    startpos = 0;
                }
                else {
                    // no match: typo or member name
                    // store the text until the last dot:
                    component_found = to_parse.substr(startpos, to_parse.rfind("."));
                    // store the complete text
                    component = to_parse.substr(startpos, std::string::npos);
                    break;
                }
            }

        // now we got the peer and taskobject pointers,
        // the completed path in peerpath
        // the last partial path in component
//         cout << "text: '" << text <<"'"<<endl;
//         cout << "to_parse: '" << text <<"'"<<endl;
//         cout << "Peerpath: '" << peerpath <<"'"<<endl;
        // cout <<endl<< "Component: '" << component <<"'"<<endl;
        // cout << "Component_found: '" << component_found <<"'"<<endl;

        RTT::TaskContext::PeerList v;
        if ( taskobject == peer->provides() ) {
            // add peer's completes:
            v = peer->getPeerList();
            for (RTT::TaskContext::PeerList::iterator i = v.begin(); i != v.end(); ++i) {
                if ( i->find( component ) == 0 ) { // only add if match
                    completes.push_back( peerpath + *i );
                    completes.push_back( peerpath + *i + "." );
                    //cerr << "added " << peerpath+*i+"."<<endl;
                }
            }
        }
        // add taskobject's completes:
        v = taskobject->getProviderNames();
        for (RTT::TaskContext::PeerList::iterator i = v.begin(); i != v.end(); ++i) {
            if ( i->find( component ) == 0 ) { // only add if match
                completes.push_back( peerpath + *i );
                if ( *i != "this" ) // "this." confuses our parsing lateron
                    completes.push_back( peerpath + *i + "." );
                //cerr << "added " << peerpath+*i+"."<<endl;
            }
        }
        // add global service completes:
        if ( peer == context && taskobject == peer->provides() ) {
            v = GlobalService::Instance()->getProviderNames();
            for (RTT::TaskContext::PeerList::iterator i = v.begin(); i != v.end(); ++i) {
                if ( i->find( component ) == 0 ) { // only add if match
                    completes.push_back( peerpath + *i );
                    if ( *i != "this" ) // "this." confuses our parsing lateron
                        completes.push_back( peerpath + *i + "." );
                    //cerr << "added " << peerpath+*i+"."<<endl;
                }
            }
        }
        return;
    }

    char ** TaskBrowser::orocos_hmi_completion ( const char *text, int start, int end )
    {
        char **matches;
        matches = ( char ** ) 0;

        matches = rl_completion_matches ( text, &TaskBrowser::command_generator );

        return ( matches );
    }
#endif // USE_READLINE

    TaskBrowser::TaskBrowser( RTT::TaskContext* _c )
        : RTT::TaskContext("TaskBrowser"),
          debug(0),
          line_read(0),
          lastc(0), storedname(""), storedline(-1),
          usehex(false),
          macrorecording(false)
    {
        tb = this;
        context = tb;
        this->switchTaskContext(_c);
#ifdef USE_READLINE
        // we always catch sigwinch ourselves, in order to pass it on to Xenomai if necessary.
#ifdef USE_SIGNALS
        rl_catch_sigwinch = 0;
#endif
        rl_completion_append_character = '\0'; // avoid adding spaces
        rl_attempted_completion_function = &TaskBrowser::orocos_hmi_completion;

        if ( read_history(".tb_history") != 0 ) {
            read_history("~/.tb_history");
        }
#ifdef USE_SIGNALS
        struct sigaction sa;
        sa.sa_sigaction = &TaskBrowser::rl_sigwinch_handler;
        sa.sa_flags = SA_SIGINFO | SA_RESTART;
        sigemptyset( &sa.sa_mask );
        sigaction(SIGWINCH, &sa, 0);
#endif // USE_SIGNALS
#endif // USE_READLINE

        this->setColorTheme( darkbg );
        this->enterTask();
    }

    TaskBrowser::~TaskBrowser() {
#ifdef USE_READLINE
        if (line_read)
            {
                free (line_read);
            }
        if ( write_history(".tb_history") != 0 ) {
            write_history("~/.tb_history");
        }
#endif
    }

    /**
     * Helper functions to display task and script states.
     */
    char getTaskStatusChar(RTT::TaskContext* t)
    {
        if (t->inFatalError())
            return 'F';
        if (t->inRunTimeError())
            return 'E';
        if (t->inException())
            return 'X';
        if (t->isRunning() )
            return 'R'; // Running
        if (t->isConfigured() )
            return 'S'; // Stopped
        return 'U';     // Unconfigured/Preoperational
    }

    char getStateMachineStatusChar(RTT::TaskContext* t, string progname)
    {
        string ps = t->getProvider<Scripting>("scripting")->getStateMachineStatusStr(progname);
        return toupper(ps[0]);
    }

    char getProgramStatusChar(RTT::TaskContext* t, string progname)
    {
        string ps = t->getProvider<Scripting>("scripting")->getProgramStatusStr(progname);
        return toupper(ps[0]);
    }

    void str_trim(string& str, char to_trim)
    {
        string::size_type pos1 = str.find_first_not_of(to_trim);
        string::size_type pos2 = str.find_last_not_of(to_trim);
        str = str.substr(pos1 == string::npos ? 0 : pos1,
                         pos2 == string::npos ? str.length() - 1 : pos2 - pos1 + 1);
    }


    /**
     * @brief Call this method from ORO_main() to
     * process keyboard input.
     */
    void TaskBrowser::loop()
    {
#ifdef USE_SIGNALS
        // Let readline intercept relevant signals
        if(rl_catch_signals == 0)
            cerr << "Error: not catching signals !"<<endl;
        if (rl_set_signals() != 0)
            cerr << "Error setting signals !" <<endl;
#endif
        cout << nl<<
            coloron <<
            "  This console reader allows you to browse and manipulate TaskContexts."<<nl<<
            "  You can type in an operation, expression, create or change variables."<<nl;
        cout <<"  (type '"<<underline<<"help"<<coloroff<<coloron<<"' for instructions and '"
        		<<underline<<"ls"<<coloroff<<coloron<<"' for context info)"<<nl<<nl;
#ifdef USE_READLINE
        cout << "    TAB completion and HISTORY is available ('bash' like)" <<nl<<nl;
#else
        cout << "    TAB completion and history is NOT available (LGPL-version)" <<nl<<nl;
#endif
        cout << "    Use '"<<underline<<"Ctrl-D"<<coloroff<<coloron<<"' or type '"<<underline<<"quit"<<coloroff<<coloron<<"' to exit this program." <<coloroff<<nl<<nl;

        while (1) {
            try {
                if (!macrorecording) {
                    if ( context == tb )
                        cout << green << " Watching " <<coloroff;

                    char state = getTaskStatusChar(taskcontext);

                    // sets prompt for readline:
//                    prompt = green + taskcontext->getName() + coloroff + "[" + state + "]> ";
                    prompt = taskcontext->getName() + " [" + state + "]> ";
                    // This 'endl' is important because it flushes the whole output to screen of all
                    // processing that previously happened, which was using 'nl'.
                    cout.flush();

                    // print traces.
                    for (PTrace::iterator it = ptraces.begin(); it != ptraces.end(); ++it) {
                        RTT::TaskContext* progpeer = it->first.first;
                        int line = progpeer->getProvider<Scripting>("scripting")->getProgramLine(it->first.second);
                        if ( line != it->second ) {
                            it->second = line;
                            printProgram( it->first.second, -1, progpeer );
                        }
                    }

                    for (PTrace::iterator it = straces.begin(); it != straces.end(); ++it) {
                        RTT::TaskContext* progpeer = it->first.first;
                        int line = progpeer->getProvider<Scripting>("scripting")->getStateMachineLine(it->first.second);
                        if ( line != it->second ) {
                            it->second = line;
                            printProgram( it->first.second, -1, progpeer );
                        }
                    }
                }
                // Check port status:
                checkPorts();
                std::string command;
                // When using rxvt on windows, the process will receive signals when the arrow keys are used
                // during input. We compile with /EHa to catch these signals and don't print anything.
                try {
#ifdef USE_READLINE
                const char* const commandStr = rl_gets();
                // quit on EOF (Ctrl-D)
                command = commandStr ? commandStr : "quit"; // copy over to string
#else
                cout << prompt;
                getline(cin,command);
                if (!cin) // Ctrl-D
                    command = "quit";
#endif
                } catch(std::exception& e) {
                    cerr << "The command line reader throwed a std::exception: '"<< e.what()<<"'."<<endl;
                } catch (...) {
                    cerr << "The command line reader throwed an exception." <<endlog();
                }
                str_trim( command, ' ');
                cout << coloroff;
                if ( command == "quit" ) {
                    // Intercept no Ctrl-C
                    cout << endl;
                    return;
                } else if ( command == "help") {
                    printHelp();
                } else if ( command.find("help ") == 0) {
                    printHelp( command.substr(command.rfind(' ')));
                } else if ( command == "#debug") {
                    debug = !debug;
                } else if ( command.find("list ") == 0 || command == "list" ) {
                    browserAction(command);
                } else if ( command.find("trace ") == 0 || command == "trace" ) {
                    browserAction(command);
                } else if ( command.find("untrace ") == 0 || command == "untrace" ) {
                    browserAction(command);
                } else if ( command.find("ls") == 0 ) {
                    std::string::size_type pos = command.find("ls")+2;
                    command = std::string(command, pos, command.length());
                    printInfo( command );
                } else if ( command == "" ) { // nop
                } else if ( command.find("cd ..") == 0  ) {
                    this->switchBack( );
                } else if ( command.find("enter") == 0  ) {
                    this->enterTask();
                } else if ( command.find("leave") == 0  ) {
                    this->leaveTask();
                } else if ( command.find("cd ") == 0  ) {
                    std::string::size_type pos = command.find("cd")+2;
                    command = std::string(command, pos, command.length());
                    this->switchTaskContext( command );
                } else if ( command.find(".") == 0  ) {
                    command = std::string(command, 1, command.length());
                    this->browserAction( command );
                } else if ( macrorecording) {
                    macrotext += command +'\n';
                } else {
                    try {
                        this->evalCommand( command );
                    } catch(std::exception& e) {
                        cerr << "The command '"<<command<<"' caused a std::exception: '"<< e.what()<<"' and could not be completed."<<endl;
                    } catch(...){
                        cerr << "The command '"<<command<<"' caused an unknown exception and could not be completed."<<endl;
                    }
                    // a command was typed... clear storedline such that a next 'list'
                    // shows the 'IP' again.
                    storedline = -1;
                }
                //cout <<endl;
            } catch(std::exception& e) {
                cerr << "Warning: The command caused a std::exception: '"<< e.what()<<"' in the TaskBrowser's loop() function."<<endl;
            } catch(...) {
                cerr << "Warning: The command caused an exception in the TaskBrowser's loop() function." << endl;
            }
         }
    }

    void TaskBrowser::enterTask()
    {
        if ( context == taskcontext ) {
            log(Info) <<"Already in Task "<< taskcontext->getName()<<endlog();
            return;
        }
        context = taskcontext;
        log(Info) <<"Entering Task "<< taskcontext->getName()<<endlog();
    }

    void TaskBrowser::leaveTask()
    {
        if ( context == tb ) {
            log(Info) <<"Already watching Task "<< taskcontext->getName()<<endlog();
            return;
        }
        context = tb;
        log(Info) <<"Watching Task "<< taskcontext->getName()<<endlog();
    }

    void TaskBrowser::recordMacro(std::string name)
    {
        if (macrorecording) {
            log(Error)<< "Macro already active." <<endlog();
            return;
        }
        if (context->provides()->hasService("scripting") == false) {
            log(Error)<< "Can not create a macro in a TaskContext without scripting service." <<endlog();
            return;
        }
        if ( name.empty() ) {
            cerr << "Please specify a macro name." <<endl;
            return;
        } else {
            cout << "Recording macro "<< name <<endl;
            cout << "Use program scripting syntax (do, set,...) !" << endl <<endl;
            cout << "export function "<< name<<" {"<<endl;
        }
        macrorecording = true;
        macroname = name;
    }

    void TaskBrowser::cancelMacro() {
        if (!macrorecording) {
            log(Warning)<< "Macro recording was not active." <<endlog();
            return;
        }
        cout << "Canceling macro "<< macroname <<endl;
        macrorecording = false;
        macrotext.clear();
    }

    void TaskBrowser::endMacro() {
        if (!macrorecording) {
            log(Warning)<< "Macro recording was not active." <<endlog();
            return;
        }
        string fname = macroname + ".ops";
        macrorecording = false;
        cout << "}" <<endl;
        cout << "Saving file "<< fname <<endl;
        ofstream macrofile( fname.c_str() );
        macrofile << "/* TaskBrowser macro '"<<macroname<<"' */" <<endl<<endl;
        macrofile << "export function "<<macroname<<" {"<<endl;
        macrofile << macrotext.c_str();
        macrofile << "}"<<endl;
        macrotext.clear();

        cout << "Loading file "<< fname <<endl;
        context->getProvider<Scripting>("Scripting")->loadPrograms(fname);
    }

    void TaskBrowser::switchBack()
    {
        if ( taskHistory.size() == 0)
            return;

        this->switchTaskContext( taskHistory.front(), false ); // store==false
        lastc = 0;
        taskHistory.pop_front();
    }

    void TaskBrowser::checkPorts()
    {
        // check periodically if the taskcontext did not change its ports.

        DataFlowInterface::Ports ports;
        ports = this->ports()->getPorts();
        for( DataFlowInterface::Ports::iterator i=ports.begin(); i != ports.end(); ++i) {
            // If our port is no longer connected, try to reconnect.
            base::PortInterface* p = *i;
            base::PortInterface* tcp = taskcontext->ports()->getPort( p->getName() );
            if ( p->connected() == false || tcp == 0 || tcp->connected() == false) {
                this->ports()->removePort( p->getName() );
                delete p;
            }
        }
    }

    void TaskBrowser::setColorTheme(ColorTheme t)
    {
        // background color palettes:
        const char* dbg = "\033[01;";
        const char* wbg = "\033[02;";
        // colors in palettes:
        const char* r = "31m";
        const char* g = "32m";
        const char* b = "34m";
        const char* con = "31m";
        const char* coff = "\33[0m";
        const char* und  = "\33[4m";

        switch (t)
            {
            case nocolors:
                green.clear();
                red.clear();
                blue.clear();
                coloron.clear();
                coloroff.clear();
                underline.clear();
                return;
                break;
            case darkbg:
                green = dbg;
                red = dbg;
                blue = dbg;
                coloron = dbg;
				coloroff = wbg;
                break;
            case whitebg:
                green = wbg;
                red = wbg;
                blue = wbg;
                coloron = wbg;
				coloroff = wbg;
                break;
            }
        green += g;
        red += r;
        blue += b;
        coloron += con;
        coloroff = coff;
        underline = und;
    }

    void TaskBrowser::switchTaskContext(std::string& c) {
        // if nothing new found, return.
        peer = taskcontext;
        if ( this->findPeer( c + "." ) == 0 ) {
            cerr << "No such peer: "<< c <<nl;
            return;
        }

        if ( peer == taskcontext ) {
            cerr << "Already in "<< c <<nl;
            return;
        }

        if ( peer == tb ) {
            cerr << "Can not switch to TaskBrowser." <<nl;
            return;
        }

        // findPeer has set 'peer' :
        this->switchTaskContext( peer );
    }

    void TaskBrowser::switchTaskContext(RTT::TaskContext* tc, bool store) {
        // put current on the stack :
        if (taskHistory.size() == 20 )
            taskHistory.pop_back();
        if ( taskcontext && store)
            taskHistory.push_front( taskcontext );

        // disconnect from current peers.
        this->disconnect();

        // cleanup port left-overs.
        DataFlowInterface::Ports tports = this->ports()->getPorts();
        for( DataFlowInterface::Ports::iterator i=tports.begin(); i != tports.end(); ++i) {
            this->ports()->removePort( (*i)->getName() );
            delete *i;
        }

        // now switch to new one :
        if ( context == taskcontext )
            context = tc;
        taskcontext = tc; // peer is the new taskcontext.
        lastc = 0;

        // connect peer.
        this->addPeer( taskcontext );

        // map data ports.
        // create 'anti-ports' to allow port-level interaction with the peer.
        tports = taskcontext->ports()->getPorts();
        if ( !tports.empty() )
            cout <<nl << "TaskBrowser connects to all data ports of "<<taskcontext->getName()<<endl;
        for( DataFlowInterface::Ports::iterator i=tports.begin(); i != tports.end(); ++i) {
            if (this->ports()->getPort( (*i)->getName() ) == 0 )
                this->ports()->addPort( *(*i)->antiClone() );
        }
        RTT::connectPorts(this,taskcontext);



        cerr << "   Switched to : " << taskcontext->getName() <<endl;

    }

    RTT::TaskContext* TaskBrowser::findPeer(std::string c) {
        // returns the one but last peer, which is the one we want.
        std::string s( c );

        our_pos_iter_t parsebegin( s.begin(), s.end(), "teststring" );
        our_pos_iter_t parseend;

        CommonParser cp;
        scripting::PeerParser pp( peer, cp, true );
        bool skipref = true;
        try {
            parse( parsebegin, parseend, pp.parser(), SKIP_PARSER );
        }
        catch( ... )
            {
                log(Debug) <<"No such peer : "<< c <<endlog();
                return 0;
            }
        taskobject = pp.taskObject();
        assert(taskobject);
        peer = pp.peer();
        return pp.peer();
    }

    void TaskBrowser::browserAction(std::string& act)
    {
        std::stringstream ss(act);
        std::string instr;
        ss >> instr;

        if ( instr == "list" ) {
            if (context->provides()->hasService("scripting") == false) {
                log(Error)<< "Can not list a program in a TaskContext without scripting service." <<endlog();
                return;
            }
            int line;
            ss >> line;
            if (ss) {
                this->printProgram(line);
                return;
            }
            ss.clear();
            string arg;
            ss >> arg;
            if (ss) {
                ss.clear();
                ss >> line;
                if (ss) {
                    // progname and line given
                    this->printProgram(arg, line);
                    return;
                }
                // only progname given.
                this->printProgram( arg );
                return;
            }
            // just 'list' :
            this->printProgram();
            return;
        }

        //
        // TRACING
        //
        if ( instr == "trace") {
            if (context->provides()->hasService("scripting") == false) {
                log(Error)<< "Can not trace a program in a TaskContext without scripting service." <<endlog();
                return;
            }

            string arg;
            ss >> arg;
            if (ss) {
                bool pi = context->getProvider<Scripting>("scripting")->hasProgram(arg);
                if (pi) {
                    ptraces[make_pair(context, arg)] = context->getProvider<Scripting>("scripting")->getProgramLine(arg); // store current line number.
                    this->printProgram( arg );
                    return;
                }
                pi = context->getProvider<Scripting>("scripting")->hasStateMachine(arg);
                if (pi) {
                    straces[make_pair(context, arg)] = context->getProvider<Scripting>("scripting")->getStateMachineLine(arg); // store current line number.
                    this->printProgram( arg );
                    return;
                }
                cerr <<"No such program or state machine: "<< arg <<endl;
                return;
            }

            // just 'trace' :
            std::vector<std::string> names;
            names = context->getProvider<Scripting>("scripting")->getProgramList();
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                bool pi = context->getProvider<Scripting>("scripting")->hasProgram(arg);
                if (pi)
                    ptraces[make_pair(context, arg)] = context->getProvider<Scripting>("scripting")->getProgramLine(arg); // store current line number.
            }

            names = context->getProvider<Scripting>("scripting")->getStateMachineList();
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                bool pi = context->getProvider<Scripting>("scripting")->hasStateMachine(arg);
                if (pi)
                    straces[make_pair(context, arg)] = context->getProvider<Scripting>("scripting")->getStateMachineLine(arg); // store current line number.
            }

            cerr << "Tracing all programs and state machines in "<< context->getName() << endl;
            return;
        }

        if ( instr == "untrace") {
            if (context->provides()->hasService("scripting") == false) {
                log(Error)<< "Can not untrace a program in a TaskContext without scripting service." <<endlog();
                return;
            }
            string arg;
            ss >> arg;
            if (ss) {
                ptraces.erase( make_pair(context, arg) );
                straces.erase( make_pair(context, arg) );
                cerr <<"Untracing "<< arg <<" of "<< context->getName()<<endl;
                return;
            }
            // just 'untrace' :
            std::vector<std::string> names;
            names = context->getProvider<Scripting>("scripting")->getProgramList();
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                bool pi = context->getProvider<Scripting>("scripting")->hasProgram(arg);
                if (pi)
                    ptraces.erase(make_pair(context, arg));
            }

            names = context->getProvider<Scripting>("scripting")->getStateMachineList();
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                bool pi = context->getProvider<Scripting>("scripting")->hasStateMachine(arg);
                if (pi)
                    straces.erase(make_pair(context, arg));
            }

            cerr << "Untracing all programs and state machines of "<< context->getName() << endl;
            return;
        }

        std::string arg;
        ss >> arg;
        if ( instr == "dark") {
            this->setColorTheme(darkbg);
            cout << nl << "Setting Color Theme for "+green+"dark"+coloroff+" backgrounds."<<endl;
            return;
        }
        if ( instr == "light") {
            this->setColorTheme(whitebg);
            cout << nl << "Setting Color Theme for "+green+"light"+coloroff+" backgrounds."<<endl;
            return;
        }
        if ( instr == "nocolors") {
            this->setColorTheme(nocolors);
            cout <<nl << "Disabling all colors"<<endl;
            return;
        }
        if ( instr == "record") {
            recordMacro( arg );
            return;
        }
        if ( instr == "cancel") {
            cancelMacro();
            return;
        }
        if ( instr == "end") {
            endMacro();
            return;
        }
        if ( instr == "hex") {
            usehex = true;
            cout << "Switching to hex notation for output (use .nohex to revert)." <<endl;
            return;
        }
        if ( instr == "nohex") {
            usehex = false;
            cout << "Turning off hex notation for output." <<endl;
            return;
        }
        if ( instr == "provide") {
            while ( ss ) {
                cout << "Trying to locate service '" << arg << "'..."<<endl;
                if ( PluginLoader::Instance()->loadService(arg, context) )
                    cout << "Service '"<< arg << "' loaded in " << context->getName() << endl;
                else
                    cout << "Service not found." <<endl;
                ss >> arg;
            }
            return;
        }
        if (instr == "services") {
            vector<string> names = PluginLoader::Instance()->listServices();
            cout << "Available Services: ";
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                cout << " " << *it;
            }
            cout <<endl;
            return;
        }
        if (instr == "typekits") {
            vector<string> names = PluginLoader::Instance()->listTypekits();
            cout << "Available Typekits: ";
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                cout << " " << *it;
            }
            cout <<endl;
            return;
        }
        if (instr == "types") {
            vector<string> names = TypeInfoRepository::Instance()->getDottedTypes();
            cout << "Available data types: ";
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                cout << " " << *it;
            }
            cout <<endl;
            return;
        }
        cerr << "Unknown Browser Action : "<< act <<endl;
        cerr << "See 'help' for valid syntax."<<endl;
    }

    void TaskBrowser::evaluate(std::string& comm) {
        this->evalCommand(comm);
    }

    Service::shared_ptr TaskBrowser::stringToService(string const& names) {
        Service::shared_ptr serv;
        std::vector<std::string> strs;
        boost::split(strs, names, boost::is_any_of("."));

        // strs could be empty because of a bug in Boost 1.44 (see https://svn.boost.org/trac/boost/ticket/4751)
        if (strs.empty()) return serv;

        string component = strs.front();
        if (! context->hasPeer(component) && !context->provides()->hasService(component) ) {
            return serv;
        }
        // We only support help for peer or subservice:
        if ( context->hasPeer(component) )
            serv = context->getPeer(component)->provides();
        else if (context->provides()->hasService(component))
            serv = context->provides(component);

        // remove component name:
        strs.erase( strs.begin() );

        // iterate over remainders:
        while ( !strs.empty() && serv) {
            serv = serv->getService( strs.front() );
            if (serv)
                strs.erase( strs.begin() );
        }
        return serv;
    }



    bool TaskBrowser::printService( string name ) {
    	bool result = false;
        Service::shared_ptr ops = stringToService(name);
        ServiceRequester* sr = 0;

        if ( ops || GlobalService::Instance()->hasService( name ) ) // only object name was typed
            {
                if ( !ops )
                    ops = GlobalService::Instance()->provides(name);
                sresult << nl << "Printing Interface of '"<< coloron << ops->getName() <<coloroff <<"' :"<<nl<<nl;
                vector<string> methods = ops->getNames();
                std::for_each( methods.begin(), methods.end(), boost::bind(&TaskBrowser::printOperation, this, _1, ops) );
                cout << sresult.str();
                sresult.str("");
                result = true;
            }
        if ( context->requires()->requiresService( name ) ) // only object name was typed
            {
                sr = context->requires(name);
                sresult << nl << "Requiring '"<< coloron << sr->getRequestName() <<coloroff <<"' with methods: ";
                vector<string> methods = sr->getOperationCallerNames();
                sresult << coloron;
                std::for_each( methods.begin(), methods.end(), sresult << lambda::_1 <<" " );
                cout << sresult.str() << coloroff << nl;
                sresult.str("");
                result = true;
            }
        return result;
    }

    void TaskBrowser::evalCommand(std::string& comm )
    {
        // deprecated: use 'help servicename'
        bool result = printService(comm);

        // Minor hack : also check if it was an attribute of current TC, for example,
        // if both the object and attribute with that name exist. the if
        // statement after this one would return and not give the expr parser
        // time to evaluate 'comm'.
        if ( context->provides()->getValue( comm ) ) {
            if (debug)
                cerr << "Found value..."<<nl;
                this->printResult( context->provides()->getValue( comm )->getDataSource().get(), true );
                cout << sresult.str()<<nl;
                sresult.str("");
                return;
        }

        if ( result ) {
            return;
        }

	    // Set caller=0 to have correct call/send semantics.
        // we're outside the updateHook(). Passing 'this' would
        // trigger the EE of the TB, but not our own function.
        scripting::Parser _parser( 0 );

        if (debug)
            cerr << "Trying ValueStatement..."<<nl;
        try {
            // Check if it was a method or datasource :
            last_expr = _parser.parseValueStatement( comm, context );
            // methods and DS'es are processed immediately.
            if ( last_expr ) {
                // only print if no ';' was given.
                assert( comm.size() != 0 );
                if ( comm[ comm.size() - 1 ] != ';' ) {
                    this->printResult( last_expr.get(), true );
                    cout << sresult.str() << nl <<endl;
                    sresult.str("");
                } else
                    last_expr->evaluate();
                return; // done here
            } else if (debug)
                cerr << "returned (null) !"<<nl;
            //cout << "    (ok)" <<nl;
            //return; //
        } catch ( fatal_semantic_parse_exception& pe ) { // incorr args, ...
            // way to fatal,  must be reported immediately
            if (debug)
                cerr << "fatal_semantic_parse_exception: ";
            cerr << pe.what() <<nl;
            return;
        } catch ( syntactic_parse_exception& pe ) { // wrong content after = sign etc..
            // syntactic errors must be reported immediately
            if (debug)
                cerr << "syntactic_parse_exception: ";
            cerr << pe.what() <<nl;
            return;
        } catch ( parse_exception_parser_fail &pe )
            {
                // ignore, try next parser
                if (debug) {
                    cerr << "Ignoring ValueStatement exception :"<<nl;
                    cerr << pe.what() <<nl;
                }
        } catch ( parse_exception& pe ) {
            // syntactic errors must be reported immediately
            if (debug)
                cerr << "parse_exception :";
            cerr << pe.what() <<nl;
            return;
        }
        if (debug)
            cerr << "Trying Expression..."<<nl;
        try {
            // Check if it was a method or datasource :
            last_expr = _parser.parseExpression( comm, context );
            // methods and DS'es are processed immediately.
            if ( last_expr ) {
                // only print if no ';' was given.
                assert( comm.size() != 0 );
                if ( comm[ comm.size() - 1 ] != ';' ) {
                    this->printResult( last_expr.get(), true );
                    cout << sresult.str() << nl << endl;
                    sresult.str("");
                } else
                    last_expr->evaluate();
                return; // done here
            } else if (debug)
                cerr << "returned (null) !"<<nl;
        } catch ( syntactic_parse_exception& pe ) { // missing brace etc
            // syntactic errors must be reported immediately
            if (debug)
                cerr << "syntactic_parse_exception :";
            cerr << pe.what() <<nl;
            return;
        } catch ( fatal_semantic_parse_exception& pe ) { // incorr args, ...
            // way to fatal,  must be reported immediately
            if (debug)
                cerr << "fatal_semantic_parse_exception :";
            cerr << pe.what() <<nl;
            return;
        } catch ( parse_exception_parser_fail &pe )
            {
                // We're the last parser!
                if (debug)
                    cerr << "Ignoring Expression exception :"<<nl;
                cerr << pe.what() <<nl;

        } catch ( parse_exception& pe ) {
                // We're the last parser!
                if (debug)
                    cerr << "Ignoring Expression parse_exception :"<<nl;
                cerr << pe.what() <<nl;
        }
    }

    void TaskBrowser::printResult( base::DataSourceBase* ds, bool recurse) {
        std::string prompt(" = ");
        // setup prompt :
        sresult <<prompt<< setw(20)<<left;
        if ( ds )
            doPrint( ds, recurse );
        else
            sresult << "(null)";
        sresult << right;
    }

    void TaskBrowser::doPrint( base::DataSourceBase::shared_ptr ds, bool recurse) {
        if (!ds) {
            sresult << "(null)";
            return;
        }

        // this is needed for ds's that rely on initialision.
        // e.g. eval true once or time measurements.
        // becomes only really handy for 'watches' (to deprecate).
        ds->reset();
        // this is needed to read a ds's value. Otherwise, a cached value may be returned.
        ds->evaluate();

        DataSource<RTT::PropertyBag>* dspbag = DataSource<RTT::PropertyBag>::narrow(ds.get());
        if (dspbag) {
            RTT::PropertyBag bag( dspbag->get() );
            if (!recurse) {
                int siz = bag.getProperties().size();
                int wdth = siz ? (20 - (siz / 10 + 1)) : 20;
                sresult <<setw(0)<< siz <<setw( wdth )<< " Properties";
            } else {
            if ( ! bag.empty() ) {
                sresult <<setw(0)<<nl;
                for( RTT::PropertyBag::iterator it= bag.getProperties().begin(); it!=bag.getProperties().end(); ++it) {
                    sresult <<setw(14)<<right<< Types()->toDot( (*it)->getType() )<<" "<<coloron<<setw(14)<< (*it)->getName()<<coloroff;
                    base::DataSourceBase::shared_ptr propds = (*it)->getDataSource();
                    this->printResult( propds.get(), false );
                    sresult <<" ("<<(*it)->getDescription()<<')' << nl;
                }
            } else {
                sresult <<prompt<<"(empty RTT::PropertyBag)";
            }
            }
            return;
        }

        // Print the members of the type:
        base::DataSourceBase::shared_ptr dsb(ds);
        if (dsb->getMemberNames().empty() || dsb->getTypeInfo()->isStreamable() ) {
            if (debug) cerr << "terminal item " << dsb->getTypeName() << nl;
            if (usehex)
                sresult << std::hex << dsb;
            else
                sresult << std::dec << dsb;
        } else {
            sresult << setw(0);
            sresult << "{";
            vector<string> names = dsb->getMemberNames();
            if ( find(names.begin(), names.end(), "capacity") != names.end() &&
                    find(names.begin(), names.end(), "size") != names.end() ) {
                // is a container/sequence:
                DataSource<int>::shared_ptr seq_size = dynamic_pointer_cast<DataSource<int> >(dsb->getMember("size"));
                if (seq_size) {
                    ValueDataSource<unsigned int>::shared_ptr index = new ValueDataSource<unsigned int>(0);
                    // print max 10 items of sequence:
                    sresult << " [";
                    for (int i=0; i != seq_size->get(); ++i) {
                        index->set( i );
                        if (i == 10) {
                            sresult << "...("<< seq_size->get() - 10 <<" items omitted)...";
                            break;
                        } else {
                            DataSourceBase::shared_ptr element = dsb->getMember(index, DataSourceBase::shared_ptr() );
                            doPrint(element, true);
                            if (i+1 != seq_size->get())
                                sresult <<", ";
                        }
                    }
                    sresult << " ], "; // size and capacity will follow...
                }
            }
            for(vector<string>::iterator it = names.begin(); it != names.end(); ) {
                sresult  << *it << " = ";
                doPrint( dsb->getMember(*it), true);
                if (++it != names.end())
                    sresult <<", ";
            }
            sresult <<" }";
        }
    }

    struct comcol
    {
        const char* command;
        comcol(const char* c) :command(c) {}
        std::ostream& operator()( std::ostream& os ) const {
            os<<"'"<< TaskBrowser::coloron<< TaskBrowser::underline << command << TaskBrowser::coloroff<<"'";
            return os;
        }
    };

    struct keycol
    {
        const char* command;
        keycol(const char* c) :command(c) {}
        std::ostream& operator()( std::ostream& os )const {
            os<<"<"<< TaskBrowser::coloron<< TaskBrowser::underline << command << TaskBrowser::coloroff<<">";
            return os;
        }
    };

    struct titlecol
    {
        const char* command;
        titlecol(const char* c) :command(c) {}
        std::ostream& operator()( std::ostream& os ) const {
            os<<endl<<"["<< TaskBrowser::coloron<< TaskBrowser::underline << command << TaskBrowser::coloroff<<"]";
            return os;
        }
    };

    std::ostream& operator<<(std::ostream& os, comcol f ){
        return f(os);
    }

    std::ostream& operator<<(std::ostream& os, keycol f ){
        return f(os);
    }

    std::ostream& operator<<(std::ostream& os, titlecol f ){
        return f(os);
    }

    void TaskBrowser::printHelp()
    {
        cout << coloroff;
        cout <<titlecol("Task Browsing")<<nl;
        cout << "  To switch to another task, type "<<comcol("cd <path-to-taskname>")<<nl;
        cout << "  and type "<<comcol("cd ..")<<" to go back to the previous task (History size is 20)."<<nl;
        cout << "  Pressing "<<keycol("tab")<<" multiple times helps you to complete your command."<<nl;
        cout << "  It is not mandatory to switch to a task to interact with it, you can type the"<<nl;
        cout << "  peer-path to the task (dot-separated) and then type command or expression :"<<nl;
        cout << "     PeerTask.OtherTask.FinalTask.countTo(3) [enter] "<<nl;
        cout << "  Where 'countTo' is a method of 'FinalTask'."<<nl;
        cout << "  The TaskBrowser starts by default 'In' the current component. In order to watch"<<nl;
        cout << "  the TaskBrowser itself, type "<<comcol("leave")<<" You will notice that it"<<nl;
        cout << "  has connected to the data ports of the visited component. Use "<<comcol("enter")<<" to enter"<<nl;
        cout << "  the visited component again. The "<<comcol("cd")<<" command works transparantly in both"<<nl;
        cout << "  modi."<<nl;

        cout << "  "<<titlecol("Task Context Info")<<nl;
        cout << "  To see the contents of a task, type "<<comcol("ls")<<nl;
        cout << "  For a detailed argument list (and helpful info) of the object's methods, "<<nl;
        cout <<"   type the name of one of the listed task objects : " <<nl;
        cout <<"      this [enter]" <<nl<<nl;
        cout <<"  factor( int number ) : bool" <<nl;
        cout <<"   Factor a value into its primes." <<nl;
        cout <<"   number : The number to factor in primes." <<nl;
        cout <<"  isRunning( ) : bool" <<nl;
        cout <<"   Is this RTT::TaskContext started ?" <<nl;
        cout <<"  loadProgram( const& std::string Filename ) : bool" <<nl;
        cout <<"   Load an Orocos Program Script from a file." <<nl;
        cout <<"   Filename : An ops file." <<nl;
        cout <<"   ..."<<nl;

        cout << "   A status character shows the TaskState of a component."<<nl;
        cout << "     'E':RunTimeError, 'S':Stopped, 'R':Running, 'U':PreOperational (Unconfigured)"<<nl;
        cout << "     'X':Exception, 'F':FatalError" << nl;

        cout <<titlecol("Expressions")<<nl;
        cout << "  You can evaluate any script expression by merely typing it :"<<nl;
        cout << "     1+1 [enter]" <<nl;
        cout << "   = 2" <<nl;
        cout << "  or inspect the status of a program :"<<nl;
        cout << "     myProgram.isRunning [enter]" <<nl;
        cout << "   = false" <<nl;
        cout << "  and display the contents of complex data types (vector, array,...) :"<<nl;
        cout << "     array(6)" <<nl;
        cout << "   = {0, 0, 0, 0, 0, 0}" <<nl;

        cout <<titlecol("Changing Attributes and Properties")<<nl;
        cout << "  To change the value of a Task's attribute, type "<<comcol("varname = <newvalue>")<<nl;
        cout << "  If you provided a correct assignment, the browser will inform you of the success"<<nl;
        cout <<"   with the set value." <<nl;

        cout <<titlecol("Operations")<<nl;
        cout << "  An Operation is sent or called (evaluated) "<<nl;
        cout << "  immediately and print the result. An example could be :"<<nl;
        cout << "     someTask.bar.getNumberOfBeers(\"Palm\") [enter] "<<nl;
        cout << "   = 99" <<nl;
        cout << "  You can ask help on an operation by using the 'help' command: "<<nl;
        cout << "     help start"<<nl;
        cout << "    start( ) : bool"<<nl;
        cout << "      Start this TaskContext (= startHook() + updateHook() )." <<nl;

        cout <<titlecol("Program and scripting::StateMachine Scripts")<<nl;
        cout << "  To load a program script use the scripting service."<<nl;
        cout << "   Use "<<comcol(".provide scripting")<< " to load the scripting service in a TaskContext."<<nl;
        cout << "  You can use "<<comcol("ls progname")<<nl;
        cout << "   to see the programs operations and variables. You can manipulate each one of these"<<nl;
        cout << "   using the service object of the program."<<nl;

        cout << "  To print a program or state machine listing, use "<<comcol("list progname [linenumber]")<<nl;
        cout << "   to list the contents of the current program lines being executed,"<<nl;
        cout << "   or 10 lines before or after <linenumber>. When only "<<comcol("list [n]")<<nl;
        cout << "   is typed, 20 lines of the last listed program are printed from line <n> on "<<nl;
        cout << "   ( default : list next 20 lines after previous list )."<<nl;

        cout << "  To trace a program or state machine listing, use "<<comcol("trace [progname]")<<" this will"<<nl;
        cout << "   cause the TaskBrowser to list the contents of a traced program,"<<nl;
        cout << "   each time the line number of the traced program changes."<<nl;
        cout << "   Disable tracing with "<<comcol("untrace [progname]")<<""<<nl;
        cout << "   If no arguments are given to "<<comcol("trace")<<" and "<<comcol("untrace")<<", it applies to all programs."<<nl;

        cout << "   A status character shows which line is being executed."<<nl;
        cout << "   For programs : 'E':Error, 'S':Stopped, 'R':Running, 'P':Paused"<<nl;
        cout << "   For state machines : <the same as programs> + 'A':Active, 'I':Inactive"<<nl;

        cout <<titlecol("Changing Colors")<<nl;
        cout << "  You can inform the TaskBrowser of your background color by typing "<<comcol(".dark")<<nl;
        cout << "  "<<comcol(".light")<<", or "<<comcol(".nocolors")<<" to increase readability."<<nl;

        cout <<titlecol("Output Formatting")<<nl;
        cout << "  Use the commands "<<comcol(".hex") << " or " << comcol(".nohex") << " to turn hexadecimal "<<nl;
        cout << "  notation of integers on or off."<<nl;

        cout <<titlecol("Macro Recording / RTT::Command line history")<<nl;
        cout << "  You can browse the commandline history by using the up-arrow key or press "<<comcol("Ctrl r")<<nl;
        cout << "  and a search term. Hit enter to execute the current searched command."<<nl;
        cout << "  Macros can be recorded using the "<<comcol(".record 'macro-name'")<<" command."<<nl;
        cout << "  You can cancel the recording by typing "<<comcol(".cancel")<<" ."<<nl;
        cout << "  You can save and load the macro by typing "<<comcol(".end")<<" . The macro becomes"<<nl;
        cout << "  available as a command with name 'macro-name' in the current TaskContext." << nl;
        cout << "  While you enter the macro, it is not executed, as you must use scripting syntax which"<<nl;
        cout << "  may use loop or conditional statements, variables etc."<<nl;

        cout <<titlecol("Connecting Ports")<<nl;
        cout << "  You can instruct the TaskBrowser to connect to the ports of the current Peer by"<<nl;
        cout << "  typing "<<comcol(".connect [port-name]")<<", which will temporarily create connections"<<nl;
        cout << "  to all ports if [port-name] is omitted or to the specified port otherwise."<<nl;
        cout << "  The TaskBrowser disconnects these ports when it visits another component, but the"<<nl;
        cout << "  created connection objects remain in place (this is more or less a bug)!"<<nl;

        cout <<titlecol("Plugins, Typekits and Services")<<nl;
        cout << "  Use "<<comcol(".provide [servicename]")<< " to load a service in a TaskContext."<<nl;
        cout << "  For example, to add XML marshalling, type: "<<comcol(".provide marshalling")<< "."<<nl;
        cout << "  Use "<<comcol(".services")<< " to get a list of available services."<<nl;
        cout << "  Use "<<comcol(".typekits")<< " to get a list of available typekits."<<nl;
        cout << "  Use "<<comcol(".types")<< " to get a list of available data types."<<nl;
    }

    void TaskBrowser::printHelp( string helpstring ) {
    	peer = context;
		// trim garbage:
		str_trim(helpstring, ' ');
		str_trim(helpstring, '.');

    	if ( printService(helpstring))
    		return;

    	if ( findPeer( helpstring ) ) {
    		try {
    		    // findPeer resolved the taskobject holding 'helpstring'.
    			sresult << nl;
    			if (helpstring.rfind('.') != string::npos )
    				printOperation( helpstring.substr(helpstring.rfind('.')+1 ), taskobject );
    			else
    				printOperation( helpstring, taskobject );
    	        cout << sresult.str();
    		} catch (...) {
        		cerr<< "  help: No such operation known: '"<< helpstring << "'"<<nl;
    		}
    	} else {
    		cerr<< "  help: No such operation known (peer not found): '"<< helpstring << "'"<<nl;
    	}
        sresult.str("");
    }

    void TaskBrowser::printProgram(const std::string& progname, int cl /*= -1*/, RTT::TaskContext* progpeer /* = 0 */) {
        string ps;
        char s;
        stringstream txtss;
        int ln;
        int start;
        int end;
        bool found(false);

        if (progpeer == 0 )
            progpeer = context;

        // if program exists, display.
        if ( progpeer->getProvider<Scripting>("scripting")->hasProgram( progname ) ) {
            s = getProgramStatusChar(progpeer, progname);
            txtss.str( progpeer->getProvider<Scripting>("scripting")->getProgramText(progname) );
            ln = progpeer->getProvider<Scripting>("scripting")->getProgramLine(progname);
            if ( cl < 0 ) cl = ln;
            start = cl < 10 ? 1 : cl - 10;
            end   = cl + 10;
            this->listText( txtss, start, end, ln, s);
            found = true;
        }

        // If statemachine exists, display.
        if ( progpeer->getProvider<Scripting>("scripting")->hasStateMachine( progname ) ) {
            s = getStateMachineStatusChar(progpeer, progname);
            txtss.str( progpeer->getProvider<Scripting>("scripting")->getStateMachineText(progname) );
            ln = progpeer->getProvider<Scripting>("scripting")->getStateMachineLine(progname);
            if ( cl < 0 ) cl = ln;
            start = cl <= 10 ? 1 : cl - 10;
            end   = cl + 10;
            this->listText( txtss, start, end, ln, s);
            found = true;
        }
        if ( !found ) {
            cerr << "Error : No such program or state machine found : "<<progname;
            cerr << " in "<< progpeer->getName() <<"."<<endl;
            return;
        }
        storedname = progname;
    }

    void TaskBrowser::printProgram(int cl /* = -1 */) {
        string ps;
        char s;
        stringstream txtss;
        int ln;
        int start;
        int end;
        bool found(false);
        if ( context->getProvider<Scripting>("scripting")->hasProgram( storedname ) ) {
            s = getProgramStatusChar(context, storedname);
            txtss.str( context->getProvider<Scripting>("scripting")->getProgramText(storedname) );
            ln = context->getProvider<Scripting>("scripting")->getProgramLine(storedname);
            if ( cl < 0 ) cl = storedline;
            if (storedline < 0 ) cl = ln -10;
            start = cl;
            end   = cl + 20;
            this->listText( txtss, start, end, ln, s);
            found = true;
        }
        if ( context->getProvider<Scripting>("scripting")->hasStateMachine(storedname) ) {
            s = getStateMachineStatusChar(context, storedname);
            txtss.str( context->getProvider<Scripting>("scripting")->getStateMachineText(storedname) );
            ln = context->getProvider<Scripting>("scripting")->getStateMachineLine(storedname);
            if ( cl < 0 ) cl = storedline;
            if (storedline < 0 ) cl = ln -10;
            start = cl;
            end   = cl+20;
            this->listText( txtss, start, end, ln, s);
            found = true;
        }
        if ( !found )
            cerr << "Error : No such program or state machine found : "<<storedname<<endl;
    }

    void TaskBrowser::listText(stringstream& txtss,int start, int end, int ln, char s) {
        int curln = 1;
        string line;
        while ( start > 1 && curln != start ) { // consume lines
            getline( txtss, line, '\n' );
            if ( ! txtss )
                break; // no more lines, break.
            ++curln;
        }
        while ( end > start && curln != end ) { // print lines
            getline( txtss, line, '\n' );
            if ( ! txtss )
                break; // no more lines, break.
            if ( curln == ln ) {
                cout << s<<'>';
            }
            else
                cout << "  ";
            cout<< setw(int(log(double(end)))) <<right << curln<< left;
            cout << ' ' << line <<endl;
            ++curln;
        }
        storedline = curln;
        // done !
    }

    void TaskBrowser::printInfo(const std::string& peerp)
    {
        // this sets this->peer to the peer given
        peer = context;
        if ( this->findPeer( peerp+"." ) == 0 ) {
            cerr << "No such peer or object: " << peerp << endl;
            return;
        }

        if ( !peer || !peer->ready()) {
            cout << nl << " Connection to peer "+peerp+" lost (peer->ready() == false)." <<endlog();
            return;
        }

        //                     	sresult << *it << "["<<getTaskStatusChar(peer->getPeer(*it))<<"] ";


        if ( peer->provides() == taskobject )
            sresult <<nl<<" Listing TaskContext "<< green << peer->getName()<<coloroff << "["<<getTaskStatusChar(peer)<<"] :"<<nl;
        else
            sresult <<nl<<" Listing Service "<< green << taskobject->getName()<<coloroff<< "["<<getTaskStatusChar(peer)<<"] :"<<nl;

		sresult <<nl<<" Configuration Properties: ";
		RTT::PropertyBag* bag = taskobject->properties();
		if ( bag && bag->size() != 0 ) {
			// Print Properties:
			for( RTT::PropertyBag::iterator it = bag->begin(); it != bag->end(); ++it) {
				base::DataSourceBase::shared_ptr pds = (*it)->getDataSource();
				sresult << nl << setw(11)<< right << Types()->toDot( (*it)->getType() )<< " "
					 << coloron <<setw(14)<<left<< (*it)->getName() << coloroff;
				this->printResult( pds.get(), false ); // do not recurse
				sresult<<" ("<< (*it)->getDescription() <<')';
			}
		} else {
			sresult << "(none)";
		}
		sresult <<nl;

        // Print "this" interface (without detail) and then list objects...
        sresult <<nl<< " Provided Interface:";

        sresult <<nl<< "  Attributes   : ";
        std::vector<std::string> objlist = taskobject->getAttributeNames();
        if ( !objlist.empty() ) {
            sresult << nl;
            // Print Attributes:
            for( std::vector<std::string>::iterator it = objlist.begin(); it != objlist.end(); ++it) {
                base::DataSourceBase::shared_ptr pds = taskobject->getValue(*it)->getDataSource();
                sresult << setw(11)<< right << Types()->toDot( pds->getType() )<< " "
                     << coloron <<setw( 14 )<<left<< *it << coloroff;
                this->printResult( pds.get(), false ); // do not recurse
                sresult <<nl;
            }
        } else {
            sresult << coloron << "(none)";
        }

        sresult <<coloroff<<nl<< "  Operations      : "<<coloron;
        objlist = taskobject->getNames();
        if ( !objlist.empty() ) {
            std::copy(objlist.begin(), objlist.end(), std::ostream_iterator<std::string>(sresult, " "));
        } else {
            sresult << "(none)";
        }
        sresult << coloroff << nl;

		sresult <<nl<< " Data Flow Ports: ";
		objlist = taskobject->getPortNames();
		if ( !objlist.empty() ) {
			for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it) {
				base::PortInterface* port = taskobject->getPort(*it);
				bool writer = dynamic_cast<OutputPortInterface*>(port) ? true : false;
				// Port type R/W
				sresult << nl << " " << ( !writer ?
					" In" : "Out");
				// Port data type + name
				if ( !port->connected() )
					sresult << "(U) " << setw(11)<<right<< Types()->toDot( port->getTypeInfo()->getTypeName() );
				else
					sresult << "(C) " << setw(11)<<right<< Types()->toDot( port->getTypeInfo()->getTypeName() );
				sresult << " "
					 << coloron <<setw( 14 )<<left<< *it << coloroff;

                InputPortInterface* iport = dynamic_cast<InputPortInterface*>(port);
                if (iport) {
                    sresult << " <= ( use '"<< iport->getName() << ".read(sample)' to read a sample from this port)";
                }
                OutputPortInterface* oport = dynamic_cast<OutputPortInterface*>(port);
                if (oport) {
                    if ( oport->keepsLastWrittenValue()) {
                    	DataSourceBase::shared_ptr dsb = oport->getDataSource();
                    	dsb->evaluate(); // read last written value.
                        sresult << " => " << dsb;
                    } else
                        sresult << " => (keepsLastWrittenValue() == false. Enable it for this port in order to see it in the TaskBrowser.)";
                }
#if 0
				// only show if we're connected to it
				if (peer == taskcontext && peer->provides() == taskobject) {
					// Lookup if we have an input with that name and
					// consume the last sample this port produced.
					InputPortInterface* iport = dynamic_cast<InputPortInterface*>(ports()->getPort(port->getName()));
					if (iport) {
						// consume sample
						iport->getDataSource()->evaluate();
						// display
						if ( peer == this)
							sresult << " <= " << DataSourceBase::shared_ptr( iport->getDataSource());
						else
							sresult << " => " << DataSourceBase::shared_ptr( iport->getDataSource());
					}
					OutputPortInterface* oport = dynamic_cast<OutputPortInterface*>(ports()->getPort(port->getName()));
					if (oport) {
						// display last written value:
						DataSourceBase::shared_ptr ds = oport->getDataSource();
						if (ds) {
							if ( peer == this)
								sresult << " => " << ds;
							else
								sresult << " <= " << ds << " (sent from TaskBrowser)";
						} else {
							sresult << "(no last written value kept)";
						}
					}
				} else {
					sresult << "(TaskBrowser not connected to this port)";
				}
#endif
				// Port description (see Service)
//                     if ( peer->provides(*it) )
//                         sresult << " ( "<< taskobject->provides(*it)->getDescription() << " ) ";
			}
		} else {
			sresult << "(none)";
		}
		sresult << coloroff << nl;

        objlist = taskobject->getProviderNames();
        sresult <<nl<< " Services: "<<nl;
        if ( !objlist.empty() ) {
            for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it)
                sresult <<coloron<< "  " << setw(14) << *it <<coloroff<< " ( "<< taskobject->provides(*it)->doc() << " ) "<<nl;
        } else {
            sresult <<coloron<< "(none)" <<coloroff <<nl;
        }

        // RTT::TaskContext specific:
        if ( peer->provides() == taskobject ) {

            objlist = peer->requires()->getOperationCallerNames();
            sresult <<nl<< " Requires Operations :";
            if ( !objlist.empty() ) {
                for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it)
                    sresult <<coloron<< "  " << *it <<coloroff << '[' << (peer->requires()->getOperationCaller(*it)->ready() ? "R]" : "!]");
                sresult << nl;
            } else {
                sresult <<coloron<< "  (none)" <<coloroff <<nl;
            }
            objlist = peer->requires()->getRequesterNames();
            sresult <<     " Requests Services   :";
            if ( !objlist.empty() ) {
                for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it)
                    sresult <<coloron<< "  " << *it <<coloroff << '[' << (peer->requires(*it)->ready() ? "R]" : "!]");
                sresult << nl;
            } else {
                sresult <<coloron<< "  (none)" <<coloroff <<nl;
            }

            if (peer->provides()->hasService("scripting")) {
                objlist = peer->getProvider<Scripting>("scripting")->getProgramList();
                if ( !objlist.empty() ) {
                    sresult << " Programs     : "<<coloron;
                    for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it)
                        sresult << *it << "["<<getProgramStatusChar(peer,*it)<<"] ";
                    sresult << coloroff << nl;
                }

                objlist = peer->getProvider<Scripting>("scripting")->getStateMachineList();
                if ( !objlist.empty() ) {
                    sresult << " StateMachines: "<<coloron;
                    for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it)
                        sresult << *it << "["<<getStateMachineStatusChar(peer,*it)<<"] ";
                    sresult << coloroff << nl;
                }
            }

            // if we are in the TB, display the peers of our connected task:
            if ( context == tb )
                sresult <<nl<< " "<<peer->getName()<<" Peers : "<<coloron;
            else
                sresult << nl <<" Peers        : "<<coloron;

            objlist = peer->getPeerList();
            if ( !objlist.empty() )
                for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it) {
                    if( peer->getPeer(*it) )
                    	sresult << *it << "["<<getTaskStatusChar(peer->getPeer(*it))<<"] ";
                    else
                    	sresult << *it << "[X] ";
	      }
            else
                sresult << "(none)";
        }
        sresult <<coloroff<<nl;
        cout << sresult.str();
        sresult.str("");
    }

    void TaskBrowser::printOperation( const std::string m, Service::shared_ptr the_ops )
    {
        std::vector<ArgumentDescription> args;
        Service::shared_ptr ops;
        try {
            args = the_ops->getArgumentList( m ); // may throw !
            ops = the_ops;
        } catch(...) {
            args = GlobalService::Instance()->getArgumentList( m ); // may throw !
            ops = GlobalService::Instance();
        }
        sresult <<" " << coloron << m << coloroff<< "( ";
        for (std::vector<ArgumentDescription>::iterator it = args.begin(); it != args.end(); ++it) {
            sresult << Types()->toDot( it->type ) <<" ";
            sresult << coloron << it->name << coloroff;
            if ( it+1 != args.end() )
                sresult << ", ";
            else
                sresult << " ";
        }
        sresult << ") : "<< Types()->toDot( ops->getResultType(m) )<<nl;
        sresult << "   " << ops->getDescription( m )<<nl;
        for (std::vector<ArgumentDescription>::iterator it = args.begin(); it != args.end(); ++it)
            sresult <<"   "<< it->name <<" : " << it->description << nl;
    }

}
