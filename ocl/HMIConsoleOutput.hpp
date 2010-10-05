/***************************************************************************
  tag: Peter Soetens  Thu Apr 22 20:40:59 CEST 2004  HMIConsoleOutput.hpp

                        HMIConsoleOutput.hpp -  description
                           -------------------
    begin                : Thu April 22 2004
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

#ifndef HMI_CONSOLE_OUTPUT_HPP
#define HMI_CONSOLE_OUTPUT_HPP

#include <rtt/TaskContext.hpp>
#include <rtt/Activity.hpp>
#include <rtt/Logger.hpp>
#include <rtt/os/MutexLock.hpp>
#include <sstream>
#include <iostream>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * @brief This component can be used to display messages on the
     * standard output.
     *
     * It is known as the 'cout' component in scripts.
     *
     * HMI == Human-Machine Interface
     */
    class OCL_API HMIConsoleOutput
        : public RTT::TaskContext
    {
        std::string coloron;
        std::string coloroff;
        std::string _prompt;
        std::ostringstream messages;
        std::ostringstream backup;
        std::ostringstream logmessages;
        std::ostringstream logbackup;

        RTT::os::Mutex msg_lock;
        RTT::os::Mutex log_lock;

    public :
        HMIConsoleOutput( const std::string& name = "cout")
            : RTT::TaskContext( name ),
              coloron("\033[1;34m"), coloroff("\033[0m"),
              _prompt("HMIConsoleOutput :\n")
        {
            this->addOperation("display", &HMIConsoleOutput::display, this, RTT::ClientThread).doc("Display a message on the console").arg("message", "The message to be displayed");
            this->addOperation("displayBool", &HMIConsoleOutput::displayBool, this, RTT::ClientThread).doc("Display a boolean on the console").arg("boolean", "The Boolean to be displayed");
            this->addOperation("displayInt", &HMIConsoleOutput::displayInt, this, RTT::ClientThread).doc("Display a integer on the console").arg("integer", "The Integer to be displayed");
            this->addOperation("displayDouble", &HMIConsoleOutput::displayDouble, this, RTT::ClientThread).doc("Display a double on the console").arg("double", "The Double to be displayed");
            this->addOperation("log", &HMIConsoleOutput::log, this, RTT::ClientThread).doc("Log a message on the console").arg("message", "The message to be logged");
            this->addOperation("logBool", &HMIConsoleOutput::logBool, this, RTT::ClientThread).doc("Log a boolean on the console").arg("boolean", "The Boolean to be logged");
            this->addOperation("logInt", &HMIConsoleOutput::logInt, this, RTT::ClientThread).doc("Log a integer on the console").arg("integer", "The Integer to be logged");
            this->addOperation("logDouble", &HMIConsoleOutput::logDouble, this, RTT::ClientThread).doc("Log a double on the console").arg("double", "The Double to be logged");

        }

        ~HMIConsoleOutput()
        {
            this->stop();
        }

        void updateHook()
        {
            {
                RTT::os::MutexLock lock1( msg_lock );
                if ( ! messages.str().empty() ) {
                    std::cout << coloron << _prompt<< coloroff <<
                        messages.str() << std::endl;
                    messages.rdbuf()->str("");
                }
            }
            {
                RTT::os::MutexLock lock1( log_lock );
                if ( ! logmessages.str().empty() ) {
                    RTT::log(RTT::Info) << logmessages.str() << RTT::endlog();
                    logmessages.rdbuf()->str("");
                }
            }
        }

        /**
         * Enable or disable using a colored prompt.
         */
        void enableColor(bool yesno = true)
        {
            if (yesno == true) {
                coloron = "\033[1;34m";
                coloroff = "\033[0m";
            } else {
                coloron.clear();
                coloroff.clear();
            }
        }

        /**
         * Set the prompt text.
         */
        void setPrompt(const std::string& prompt)
        {
            _prompt = prompt;
        }


        /**
         * @brief Display a message on standard output.
         */
        void display(const std::string & what)
        {
            this->enqueue( what );
        }

        /**
         * Put a message in the queue.
         * The message must be convertible to a stream using
         * operator<<().
         */
        template<class T>
        void enqueue( const T& what )
        {
            {
                RTT::os::MutexTryLock try_lock( msg_lock );
                if ( try_lock.isSuccessful() ) {
                    // we got the lock, copy everything...
                    messages << backup.str();
                    messages << what << std::endl;
                    backup.rdbuf()->str("");
                }
                else  // no lock, backup.
                    backup << what << std::endl;
                // support for non periodic logging:
            }
            if ( this->engine()->getActivity() )
                this->engine()->getActivity()->trigger();
        }

        /**
         * @brief Display a boolean on standard output.
         */
        void displayBool(bool what)
        {
            this->enqueue( what );
        }

        /**
         * @brief Display an integer on standard output.
         */
        void displayInt( int what)
        {
            this->enqueue( what );
        }

        /**
         * @brief Display a double on standard output.
         */
        void displayDouble( double what )
        {
            this->enqueue( what );
        }

        template<class T>
        void dolog( const T& what )
        {
            {
                RTT::os::MutexTryLock try_lock( log_lock );
                if ( try_lock.isSuccessful() ) {
                    // we got the lock, copy everything...
                    logmessages << logbackup.str();
                    logmessages << what;
                    logbackup.rdbuf()->str("");
                }
                else  // no lock, backup.
                    logbackup << what;
            }
            if ( this->engine()->getActivity() )
                this->engine()->getActivity()->trigger();
        }


        void log(const std::string & what)
        {
            this->dolog( what );
        }
        /**
         * @brief Log a boolean on standard output.
         */
        void logBool(bool what)
        {
            this->dolog( what );
        }

        /**
         * @brief Log an integer on standard output.
         */
        void logInt( int what)
        {
            this->dolog( what );
        }

        /**
         * @brief Log a double on standard output.
         */
        void logDouble( double what )
        {
            this->dolog( what );
        }

    };

}

#endif
