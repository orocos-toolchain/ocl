#ifndef NO_GPL
/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:31:33 CEST 2008  TaskBrowser.hpp

                        TaskBrowser.hpp -  description
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
  tag: Peter Soetens  Tue Dec 21 22:43:08 CET 2004  TaskBrowser.hpp

                        TaskBrowser.hpp -  description
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

#ifndef ORO_TASKBROWSER_HPP
#define ORO_TASKBROWSER_HPP


#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Service.hpp>
#include <deque>
#include <string>
#include <sstream>
#include <vector>
#if defined(HAS_READLINE) && !defined(HAS_EDITLINE) && defined(_POSIX_VERSION)
#include <signal.h>
#endif

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * @brief This component allows a text client to browse the
     * peers of a peer RTT::TaskContext and execute commands.
     * If your console does not support colors or you want a different
     * prompt, the member variables which control these 'escape sequences'
     * are public and may be changed. The TaskBrowser is most commonly used
     * with its loop() method, but prior to/after calling loop(), you can
     * invoke some other commands, to control what is displayed or to
     * execute a fixed set of commands prior to showng the prompt.
     */
    class OCL_API TaskBrowser
        : public RTT::TaskContext
    {
        // the 'current' task context
        static RTT::TaskContext* taskcontext;
        // the TC we are using for completion.
        static RTT::TaskContext* peer;
        // the TaskBrowser
        static RTT::TaskContext* tb;
        // the current Context: is tb or taskcontext
        static RTT::TaskContext* context;
        static RTT::Service::shared_ptr taskobject;
        RTT::internal::DataSource<bool>::shared_ptr   accepted;

        int debug;
        /* A static variable for holding the line. */
        char *line_read;
        int lastc; // last command's number

        std::string storedname; //! last program listed to screen
        int storedline; //!last program line number listed to screen
        bool usehex;

        std::deque<RTT::TaskContext*> taskHistory;

        // store TC + program/state name + line number
        typedef std::map<std::pair<RTT::TaskContext*,std::string>,int> PTrace;
        PTrace ptraces;
        PTrace straces;

        //file to store history
        const char* histfile;

        //! We store the last parsed expression in order to keep
        //! it a little longer in memory, for example, when it's an 'send()' operation call.
        base::DataSourceBase::shared_ptr last_expr;
#if defined(HAS_READLINE) || defined(HAS_EDITLINE)
#if defined(_POSIX_VERSION) && !defined(HAS_EDITLINE)
        static void rl_sigwinch_handler(int sig, siginfo_t *si, void *ctxt);
#endif

        static int rl_received_signal;
        static void rl_signal_handler(int sig, siginfo_t *si, void *ctxt);

        /* Custom implementation of rl_getc() to handle signals correctly. */
        static int rl_getc(FILE *);

        /* Read a string, and return a pointer to it.
           Returns NULL on EOF. */
        char *rl_gets ();

        // use this vector to generate candidate strings
        static std::vector<std::string> candidates;
        // Add successful matches of candidate strings to completes.
        static std::vector<std::string> completes;
        static std::vector<std::string>::iterator complete_iter;

        static std::string component_found;
        static std::string component;
        static std::string peerpath;
        static std::string text;

        // helper function
        static char* dupstr( const char *s );

        static void find_completes();

        static void find_ops();
        static void find_peers(std::string::size_type startpos);

        static char ** orocos_hmi_completion ( const char *text, int start, int end );

        static char *command_generator( const char *_text, int state );
#endif
        static RTT::TaskContext* findPeer( std::string comm );

    protected:

        void listText(std::stringstream& txtss,int start, int end, int ln, char s);

        void doPrint( RTT::base::DataSourceBase::shared_ptr ds, bool recurse);

        void enterTask();
        void leaveTask();

        bool macrorecording;
        std::string macrotext;
        std::string macroname;
        std::stringstream sresult;
        void recordMacro(std::string name);
        void cancelMacro();
        void endMacro();

        void checkPorts();
        Service::shared_ptr stringToService(std::string const& names);
    public:

        /**
         * The kinds of color themes the TaskBrowser supports.
         */
        enum ColorTheme { nocolors, //!< Do not use colors
                          darkbg, //!< Use colors suitable for a dark background
                          whitebg  //!< Use colors suitable for a white background
        };

        void setColorTheme( ColorTheme t );

        /**
         * Switch to a peer RTT::TaskContext using a \a path . For
         * example, "mypeer.otherpeer.targetpeer".
         * @param path The path to the target peer.
         */
        void switchTaskContext(std::string& path);

        /**
         * Connect the TaskBrowser to another Taskcontext.
         * @param tc the new Task to connect to
         * @param store set to \a true in order to store the \b current task
         * in the history.
         */
        void switchTaskContext( RTT::TaskContext* tc, bool store = true);

        /**
         * Go to the previous peer in the visit history.
         */
        void switchBack();

        /**
         * Execute a specific browser action, such as
         * "loadProgram pname", "loadStateMachine smname", "unloadProgram pname",
         * "unloadStateMachine smname"
         */
        void browserAction(std::string& act );

        /**
         * Evaluate a internal::DataSource and print the result.
         */
        void printResult( RTT::base::DataSourceBase* ds, bool recurse);

        /**
         * Print the help page.
         */
        void printHelp();

        /**
         * Print help about an operation
         */
        void printHelp(std::string command);

        /**
         * Print info this peer or another peer at "peerpath".
         */
        void printInfo(const std::string& peerpath);

        /**
         * Print the synopsis of a DataSource.
         */
        void printSource( const std::string m );

        /**
         * Print the synopsis of an Operation
         */
        void printOperation( const std::string m, Service::shared_ptr ops );

        /**
         * Print the synopsis of a Service.
         */
        bool printService( const std::string name);

        /**
         * Print GlobalsRepository attributes and constants.
         */
        bool printGlobals();

        /**
         * Print a program listing of a loaded program centered at line \a line.
         */
        void printProgram( const std::string& pn, int line = -1, RTT::TaskContext* progpeer = 0 );

        /**
         * Print the program listing of the last shown program centered at line \a line.
         */
        void printProgram( int line = -1 );

        /**
         * Create a TaskBrowser which initially visits a given
         * RTT::TaskContext \a c.
         */
        TaskBrowser( RTT::TaskContext* c );

        ~TaskBrowser();

        /**
         * @brief Call this method from ORO_main() to
         * process keyboard input and thus startup the
         * TaskBrowser.
         */
        void loop();

        /**
         * Execute/evaluate a string which may be a command,
         * method, expression etc.
         * The string does not need the script prefixes such as 'do', 'set', etc.
         * For example "x = 1 + 1" or "myobject.doStuff(1, 2, true)".
         */
        void evaluate(std::string& comm );

        /**
         * Switch to a new TaskContext.
         */
        void switchTask( RTT::TaskContext* c);

        /**
         * The prompt.
         */
        static std::string prompt;
        /**
         * The 'turn color on' escape sequence.
         */
        static std::string coloron;
        /**
         * The 'underline' escape sequence.
         */
        static std::string underline;
        /**
         * The 'turn color off' escape sequence.
         */
        static std::string coloroff;

        /**
         * The red color.
         */
        static std::string red;

        /**
         * The green color.
         */
        static std::string green;

        /**
         * The blue color.
         */
        static std::string blue;

        /**
         * Evaluate command
         */
        void evalCommand(std::string& comm );


    };

}

#endif
