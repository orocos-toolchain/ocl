#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <taskbrowser/TaskBrowser.hpp>
#include <deployment/DeploymentComponent.hpp>
#include <iostream>
#include <string>
#include <functional>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace po = boost::program_options;

// map lowercase strings to levels
std::map<std::string, RTT::Logger::LogLevel>	logMap = 
	boost::assign::map_list_of
	("never",		RTT::Logger::Debug)
	("fatal",		RTT::Logger::Fatal)
	("critical",	RTT::Logger::Critical)
	("error", 		RTT::Logger::Error)
	("warning", 	RTT::Logger::Warning)
	("info", 		RTT::Logger::Info)
	("debug", 		RTT::Logger::Debug)
	("realtime",	RTT::Logger::RealTime);


int ORO_main(int argc, char** argv)
{
	std::string				script;
	std::string				name;
	std::string				logLevel	= "info";	// set to valid default
	
	po::options_description 			desc("Allowed options");
	po::positional_options_description 	pos;
	desc.add_options()
		("help", 
		 "Show program usage")
		("start", 
		 po::value<std::string>(&script)->default_value("config-file.xml"),
		 "Deployment configuration file")
		("log-level", 
		 po::value<std::string>(&logLevel)->default_value("Info"),
		 "Level at which to log (case-insensitive) Never,Fatal,Critical,Error,Warning,Info,Debug,Realtime")
		("DeployerName", 
		 po::value<std::string>(&name)->default_value("Deployer"),
		 "Name of deployer component")
		;
	pos.add("DeployerName", 1);

	po::variables_map vm;
	try 
        {
            po::store(po::command_line_parser(argc, argv).
                      options(desc).positional(pos).run(), 
                      vm);
            po::notify(vm);    
            if (vm.count("help")) 
                {
                    std::cout << desc << std::endl;
                    return 1;
                }
            boost::algorithm::to_lower(logLevel);	// always lower case
            // verify that is a valid logging level
            if (vm.count("log-level") &&
                (0 == logMap.count(logLevel)))
                {
                    std::cout << "Did not understand log level: " 
                              << logLevel << std::endl 
                              << desc << std::endl;
                    return -1;
                }
        }
	catch (std::logic_error e) 
        {
            std::cerr << "Exception:" << std::endl << e.what() << std::endl;
            return -1;
        }	

	RTT::Logger::Instance()->setLogLevel(logMap[logLevel]);
    OCL::DeploymentComponent dc( name );

    if ( !script.empty() )
        {
            dc.kickStart( script );
        }

    OCL::TaskBrowser tb( &dc );

    tb.loop();

    return 0;
}
