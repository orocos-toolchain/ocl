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
#include <rtt/rtt-config.h>

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
	\param argv Command line arguments
	\param siteFile Name of the site CPF/XML file for the deployer to load up front
	\param scriptFiles Names of the XML files for the deployer to load and run
	\param name Name of the deployer task
    \param requireNameService Whether to require the CORBA name service, or not
	\param minNumberCPU The minimum number of CPUs required for deployment (0 == no minimum)
    \param vm The variables map into which the options are parsed.
	\param otherOptions Caller can pass in other options to check for. If NULL,
	then is ignored.
	\return 0 if successful, otherwise an error code
*/
extern int deployerParseCmdLine(
	int                                             argc,
	char**                                          argv,
	std::string&                                    siteFile,
	std::vector<std::string>&                       scriptFiles,
	std::string&                                    name,
    bool&                                           requireNameService,
    bool&                                           deploymentOnlyChecked,
	int&											minNumberCPU,
    boost::program_options::variables_map&          vm,
	boost::program_options::options_description*    otherOptions=NULL);


/** Enforce a minimum number of CPUs required for deployment
	\pre 0 <= minNumberCPU
	\post If 0 < \a minNumberCPU and the number of CPUs present < minNumberCPU
	then prints an error message and exit's with -1. Otherwise, nothing happens.
	\warning Check only occurs on gnulinux (the only one with RTT CPU affinity
	support)
	\return 0 if (0==minNumberCPU) or (number CPUS <= minNumberCPU) or
	the platform does not support CPU affinity in RTT, otherwise
	-1 if unable to determine the number of CPUS, otherwise -2 (and so the
	minimum number of CPUs is not present)
*/
extern int enforceMinNumberCPU(const int minNumberCPU);

#ifdef  ORO_BUILD_RTALLOC

/** Represents a memory size in bytes.
    The custom type allows use of boost::program_options's built-in
    custom validator support.
*/
struct memorySize
{
public:
    memorySize() :
            size(0)
    {}
    memorySize(size_t s) :
            size(s)
    {}
    size_t  size;
};

/** Stream \a m onto \a os
    Required for default_value support in boost::program_options
*/
inline std::ostream& operator<<(std::ostream& os, memorySize m)
{
    os << m.size;
    return os;
}

/** Get command line options for real-time memory allocation
 */
extern boost::program_options::options_description deployerRtallocOptions(
    memorySize& rtallocMemorySize);

#if     defined(ORO_BUILD_LOGGING) && defined(OROSEM_LOG4CPP_LOGGING)
/** Get command line options for log4cpp-configuration of RTT category
 */
extern boost::program_options::options_description deployerRttLog4cppOptions(
    std::string& rttLog4cppConfigFile);

/** Configure where RTT::Logger's file logging goes to (which is through
 *  log4cpp, but not through OCL's log4cpp-derived real-time logging!)
 *  Does not affect the priority of the RTT::Logger::log4cppCategoryName category.
 *
 *  @param rttLog4cppConfigFile Name of file to read log4cpp configuration from.
 *  If empty, then configures a default file appender to 'orocos.log'
 *  @return true if successfully setup the configuration
 */
extern int deployerConfigureRttLog4cppCategory(const std::string& rttLog4cppConfigFile);

#endif

#endif

// namespace
}

#endif
