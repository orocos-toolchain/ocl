/***************************************************************************
 Copyright (c) 2008 S Roderick <xxxkiwi DOT xxxnet AT macxxx DOT comxxx>
                               (remove the x's above)

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
 ***************************************************************************/

#ifndef ORO_DEPLOYER_FUNCS_HPP
#define ORO_DEPLOYER_FUNCS_HPP

#include <string>
#include <boost/program_options.hpp>

namespace OCL
{

/** Parse the command line arguments for a deployer program
	The caller can set defaults for @script and @name prior to calling
	this function.
	If the user requests a log level for RTT::Logger, then the logging level
	is set internally by this function.
	If the user requests help, then the function displays the help and
	returns a non-0 value.
	Any error causes the program usage to be displayed, and a non-0 return value.

	\param argc Number of command line arguments
	\param argv RTT::Command line arguments
	\param script Name of the XML file for the deployer to load and run
	\param name Name of the deployer task
    \param requireNameService Whether to require the CORBA name service, or not
    \param vm The variables map into which the options are parsed.
	\param otherOptions Caller can pass in other options to check for. If NULL,
	then is ignored.
	\return 0 if successful, otherwise an error code
*/
extern int deployerParseCmdLine(
	int                                             argc,
	char**                                          argv,
	std::string&                                    script,
	std::string&                                    name,
    bool&                                           requireNameService,
    boost::program_options::variables_map&          vm,
	boost::program_options::options_description*    otherOptions=NULL);

// namespace
}

#endif
