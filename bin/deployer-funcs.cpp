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

#include "deployer-funcs.hpp"
#include <rtt/Logger.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <functional>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace po = boost::program_options;

using namespace RTT;

namespace OCL
{

// map lowercase strings to levels
std::map<std::string, Logger::LogLevel>	logMap =
	boost::assign::map_list_of
	("never",       Logger::Debug)
	("fatal",       Logger::Fatal)
	("critical",    Logger::Critical)
	("error",       Logger::Error)
	("warning",     Logger::Warning)
	("info",        Logger::Info)
	("debug",       Logger::Debug)
	("realtime",    Logger::RealTime);

int deployerParseCmdLine(int                        argc,
                         char**                     argv,
                         std::string&               script,
                         std::string&               name,
                         bool&                      requireNameService,
                         po::variables_map&         vm,
                         po::options_description*   otherOptions)
{
	std::string                         logLevel("info");	// set to valid default

	po::options_description             options;
	po::options_description             allowed("Allowed options");
	po::positional_options_description  pos;
	allowed.add_options()
		("help,h",
		 "Show program usage")
		("start,s",
		 po::value<std::string>(&script),
		 "Deployment configuration file (eg 'config-file.xml')")
		("log-level,l",
		 po::value<std::string>(&logLevel),
		 "Level at which to log (case-insensitive) Never,Fatal,Critical,Error,Warning,Info,Debug,Realtime")
		("no-consolelog",
		 "Turn off logging to the console (will still log to 'orocos.log')")
        ("require-name-service",
         "Require CORBA name service")
		("DeployerName",
		 po::value<std::string>(&name),
		 "Name of deployer component (the --DeployerName flag is optional)")
		;
	pos.add("DeployerName", 1);

	// collate options
	options.add(allowed);
	if (NULL != otherOptions)
	{
		options.add(*otherOptions);
	}

	try
	{
		po::store(po::command_line_parser(argc, argv).
                  options(options).positional(pos).run(),
                  vm);
		po::notify(vm);

        // deal with options
		if (vm.count("help"))
		{
			std::cout << options << std::endl;
			return 1;
		}

        // turn off all console logging
		if (vm.count("no-consolelog"))
		{
            Logger::Instance()->mayLogStdOut(false);
            log(Warning) << "Console logging disabled" << endlog();
		}

		if (vm.count("require-name-service"))
		{
            requireNameService = true;
            log(Info) << "CORBA name service required." << endlog();
		}

 		// verify that is a valid logging level
		boost::algorithm::to_lower(logLevel);	// always lower case
		if (vm.count("log-level"))
		{
			if (0 != logMap.count(logLevel))
			{
				Logger::Instance()->setLogLevel(logMap[logLevel]);
			}
			else
			{
				std::cout << "Did not understand log level: "
						  << logLevel << std::endl
						  << options << std::endl;
				return -1;
			}
		}
	}
	catch (std::logic_error e)
    {
		std::cerr << "Exception:" << e.what() << std::endl << options << std::endl;
        return -1;
    }

    return 0;
}

// namespace
}
