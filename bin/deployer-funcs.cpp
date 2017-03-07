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
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <functional>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#ifdef  ORO_BUILD_RTALLOC
// need access to all TLSF functions embedded in RTT
#define ORO_MEMORY_POOL
#define _DEBUG_TLSF_ 1
#include <rtt/os/tlsf/tlsf.h>
#include <boost/date_time/posix_time/posix_time.hpp>    // with I/O
#include <fstream>
#endif

#if		defined(ORO_SUPPORT_CPU_AFFINITY)
#include <unistd.h>
#endif

#if     defined(ORO_BUILD_LOGGING) && defined(OROSEM_LOG4CPP_LOGGING)
// to configure RTT's use of log4cpp
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/PropertyConfigurator.hh>
#endif

namespace po = boost::program_options;
#ifdef  ORO_BUILD_RTALLOC
namespace bpt	= boost::posix_time;
#endif

using namespace RTT;
using namespace std;

#define ORO_xstr(s) ORO_str(s)
#define ORO_str(s) #s

namespace OCL
{

// map lowercase strings to levels
std::map<std::string, RTT::Logger::LogLevel>	logMap =
	boost::assign::map_list_of
	("never",       RTT::Logger::Debug)
	("fatal",       RTT::Logger::Fatal)
	("critical",    RTT::Logger::Critical)
	("error",       RTT::Logger::Error)
	("warning",     RTT::Logger::Warning)
	("info",        RTT::Logger::Info)
	("debug",       RTT::Logger::Debug)
	("realtime",    RTT::Logger::RealTime);

int deployerParseCmdLine(int                        argc,
                         char**                     argv,
                         std::string&               siteFile,
                         std::vector<std::string>&  scriptFiles,
                         std::string&               name,
                         bool&                      requireNameService,
                         bool&						deploymentOnlyChecked,
						 int&						minNumberCPU,
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
		("version",
		 "Show program version")
		("daemon,d",
		 "Makes this program a daemon such that it runs in the background. Returns 1 in case of success.")
		("start,s",
		 po::value< std::vector<std::string> >(&scriptFiles),
         "Deployment XML or script file (eg 'config-file.xml', 'script.ops' or 'script.lua')")
		("site-file",
		 po::value<std::string>(&siteFile),
		 "Site deployment XML file (eg 'Deployer-site.cpf' or 'Deployer-site.xml')")
		("log-level,l",
		 po::value<std::string>(&logLevel),
		 "Level at which to log from RTT (case-insensitive) Never,Fatal,Critical,Error,Warning,Info,Debug,Realtime")
		("no-consolelog",
		 "Turn off RTT logging to the console (will still log to 'orocos.log')")
		("check",
		 "Only check component loading, connecting peers and ports. Returns 255 in case of errors.")
        ("require-name-service",
         "Require CORBA name service")
		("minNumberCPU",
		 po::value<int>(&minNumberCPU),
		 "The minimum number of CPUs required for deployment (0 <= value) [0==no minimum (default)]")
		("DeployerName",
		 po::value< std::vector<std::string> >(),
		 "Name of deployer component (the --DeployerName flag is optional). If you provide a script or XML file name, that will be run instead.")
		;
    pos.add("DeployerName", -1);

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

        // version info
		if (vm.count("version"))
		{
            std::cout<< " OROCOS Toolchain version '" ORO_xstr(RTT_VERSION) "'";
#ifdef __GNUC__
            std::cout << " ( GCC " ORO_xstr(__GNUC__) "." ORO_xstr(__GNUC_MINOR__) "." ORO_xstr(__GNUC_PATCHLEVEL__) " )";
#endif
#ifdef OROPKG_OS_LXRT
            std::cout<<" -- LXRT/RTAI.";
#endif
#ifdef OROPKG_OS_GNULINUX
            std::cout<<" -- GNU/Linux.";
#endif
#ifdef OROPKG_OS_XENOMAI
            std::cout<<" -- Xenomai.";
#endif
			std::cout << endl;
			return 1;
		}

		if (vm.count("daemon"))
		{
			if (vm.count("check"))
				log(Warning) << "--check and --daemon are incompatible. Skipping the --daemon flag." <<endlog();
			else
				if (fork() != 0 )
					return 1;
		}

		if ( !(0 <= minNumberCPU) )
		{
			std::cout << std::endl
            << "ERROR: Invalid minimum number CPU. Require 0 <= value"
            << std::endl << options << std::endl;
			return -2;
		}

		// turn off all console logging
		if (vm.count("no-consolelog"))
		{
			RTT::Logger::Instance()->mayLogStdOut(false);
			log(Info) << "Console logging disabled" << endlog();
		}

		if (vm.count("check"))
		{
			deploymentOnlyChecked = true;
			log(Info) << "Deployment check: Only check component loading, connecting peers and ports. Returns 255 in case of errors." << endlog();
		}

		if (vm.count("require-name-service"))
		{
			requireNameService = true;
			log(Info) << "CORBA name service required." << endlog();
		}

 		// verify that is a valid logging level
		logLevel = boost::algorithm::to_lower_copy(logLevel);	// always lower case
		if (vm.count("log-level"))
		{
			if (0 != logMap.count(logLevel))
			{
				RTT::Logger::Instance()->setLogLevel(logMap[logLevel]);
			}
			else
			{
				std::cout << "Did not understand log level: "
						  << logLevel << std::endl
						  << options << std::endl;
				return -1;
			}
		}
		if (vm.count("DeployerName")) {
            const std::vector<std::string> &positional_arguments = vm["DeployerName"].as< std::vector<std::string> >();
            for(std::vector<std::string>::const_iterator it = positional_arguments.begin(); it != positional_arguments.end(); ++it) {
                if (it->size() >= 1 && it->at(0) == '_') continue; // ignore arguments that start with a _
                std::string arg = *it;
                if (arg.rfind(".xml") != std::string::npos ||
                    arg.rfind(".cpf") != std::string::npos ||
                    arg.rfind(".osd") != std::string::npos ||
                    arg.rfind(".ops") != std::string::npos ||
                    arg.rfind(".lua") != std::string::npos ) {
                    scriptFiles.push_back(arg);
                } else {
                    name = arg;
                }
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

int enforceMinNumberCPU(const int minNumberCPU)
{
	assert(0 <= minNumberCPU);

	// enforce min CPU constraints
	if (0 < minNumberCPU)
	{
#if		defined(ORO_SUPPORT_CPU_AFFINITY)
#ifdef	__linux__
		int numCPU = sysconf( _SC_NPROCESSORS_ONLN );
#else
#error Unsupported configuration!
#endif
		if (-1 == numCPU)
		{
			std::cerr << "ERROR: Unable to determine the number of CPUs for minimum number CPU support."
					  << std::endl;
			return -1;
		}
		else if (numCPU < minNumberCPU)
		{
			std::cerr << "ERROR: Number of CPUS (" << numCPU
					  << ") is less than minimum required (" << minNumberCPU
					  << ")" << std::endl;
			return -2;
		}
		// else ok as numCPU <= minNumberCPU

#else
		std::cout << "WARNING: Ignoring minimum number of CPU requirement "
				  << "as RTT does not support CPU affinity on this platform."
				  << std::endl;
#endif
	}

	return 0;
}

#ifdef  ORO_BUILD_RTALLOC

void validate(boost::any& v,
              const std::vector<std::string>& values,
              memorySize* target_type, int)
{
//    using namespace boost::program_options;

    // Make sure no previous assignment to 'a' was made.
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& memSize = po::validators::get_single_string(values);

    /* parse the string. Support "number" or "numberX" where
       X is one of {k,K,m,M,g,G} with the expected meaning of X

       e.g.

       1024, 1k, 3.4M, 4.5G
    */
    float       value=0;
    char        units='\0';
    if (2 == sscanf(memSize.c_str(), "%f%c", &value, &units))
    {
        float       multiplier=1;
        if (islower(units))
        {
            units = toupper(units);
        }
        switch (units)
        {
            case 'G':
                multiplier *= 1024;
                // fall through
            case 'M':
                multiplier *= 1024;
                // fall through
            case 'K':
                multiplier *= 1024;
                break;
            default:
                std::stringstream   e;
                e << "Invalid units in rtalloc-mem-size option: " <<  memSize 
                  << ". Valid units: 'k','m','g' (case-insensitive).";
                throw po::invalid_option_value(e.str());
        }
        value *= multiplier;
    }
    else if (1 == sscanf(memSize.c_str(), "%f", &value))
    {
        // nothing to do
    }
    else
    {
        throw po::invalid_option_value("Could not parse rtalloc-mem-size option: " + memSize);
    }

    // provide some basic sanity checking
    // Note that TLSF has its own internal checks on the value.
    // TLSF's minimum size varies with build options, but it is
    // several kilobytes at least (6-8k on Mac OS X Snow Leopard
    // with 64-bit build, ~3k on Ubuntu Jaunty with 32-bit build).
    if (! (0 <= value) )
    {
        std::stringstream   e;
        e << "Invalid memory size of " << value << " given. Value must be >= 0.";
        throw po::invalid_option_value(e.str());
    }

    v = memorySize((size_t)value);
}


boost::program_options::options_description deployerRtallocOptions(memorySize& rtallocMemorySize)
{
    boost::program_options::options_description rtallocOptions("Real-time memory allocation options");
    rtallocOptions.add_options()
		("rtalloc-mem-size",
         po::value<memorySize>(&rtallocMemorySize)->default_value(rtallocMemorySize),
		 "Amount of memory to provide for real-time memory allocations (e.g. 10000, 1K, 4.3m)\n"
         "NB the minimum size depends on TLSF build options, but it is several kilobytes.")
        ;
    return rtallocOptions;
}

TLSFMemoryPool::TLSFMemoryPool() :
    rtMem(0)
{}

TLSFMemoryPool::~TLSFMemoryPool()
{
    shutdown();
}

bool TLSFMemoryPool::initialize(const size_t memSize)
{
    if (0 != rtMem) return false;     // avoid double init
    if (0 >= memSize) return false;   // invalid size

    // don't calloc() as is first thing TLSF does.
    rtMem = malloc(memSize);
    assert(0 != rtMem);
    const size_t freeMem = init_memory_pool(memSize, rtMem);
    if ((size_t)-1 == freeMem)
    {
        std::cerr << std::dec
                  << "Invalid memory pool size of " << memSize
                  << " bytes (TLSF has a several kilobyte overhead)." << std::endl;
        free(rtMem);
        rtMem   = 0;
        return false;
    }
    std::cout << std::dec
              << "Real-time memory: " << freeMem << " bytes free of "
              << memSize << " allocated." << std::endl;
    return true;
}

void TLSFMemoryPool::shutdown()
{
    if (0 != rtMem)
    {
        // only dump TLSF debug if env. var. is set
        if (NULL != getenv("ORO_DEPLOYER_DUMP_TLSF_ON_SHUTDOWN"))
        {
            OCL::deployerDumpTLSF();
        }

        const size_t overhead = get_overhead_size(rtMem);
        std::cout << std::dec
                  << "TLSF bytes allocated=" << get_pool_size(rtMem)
                  << " overhead=" << overhead
                  << " max-used=" << (get_max_size(rtMem) - overhead)
                  << " still-allocated=" << (get_used_size(rtMem) - overhead)
                  << std::endl;

        destroy_memory_pool(rtMem);
        free(rtMem);
        rtMem   = 0;
    }
}

void deployerDumpTLSF()
{
    std::ofstream file;

    // format now as "YYYYMMDDTHHMMSS.ffffff"
    const bpt::ptime now = bpt::microsec_clock::local_time();
    static const size_t START = strlen("YYYYMMDDTHHMMSS,");
    std::string now_s = bpt::to_iso_string(now);    // YYYYMMDDTHHMMSS,fffffffff
    now_s.replace(START-1, 1, ".");                 // replace "," with "."
    now_s.erase(now_s.size()-3, 3);                 // from nanosec to microsec

    // TLSF debug
    FILE* ff = fopen("deployer_tlsf_dump.txt", "a");     // write+append
    if (0 != ff)
    {
        fprintf(ff, "# Log at %s\n", now_s.c_str());
        print_all_blocks_mp(ff);
        (void)fclose(ff);
        ff=0;
    }
}

#endif  //  ORO_BUILD_RTALLOC


#if     defined(ORO_BUILD_LOGGING) && defined(OROSEM_LOG4CPP_LOGGING)

boost::program_options::options_description deployerRttLog4cppOptions(std::string& rttLog4cppConfigFile)
{
    po::options_description     rttLog4cppOptions("RTT/Log4cpp options");
    rttLog4cppOptions.add_options()
		("rtt-log4cpp-config-file",
         po::value<std::string>(&rttLog4cppConfigFile),
		 std::string("Log4cpp configuration file to support RTT category '" + RTT::Logger::log4cppCategoryName + "'\n"+
                     "WARNING Configure only this category. Use deployment files to configure realtime logging!").c_str())
        ;
    return rttLog4cppOptions;
}

int deployerConfigureRttLog4cppCategory(const std::string& rttLog4cppConfigFile)
{
    // configure where RTT::Logger's file logging goes to (which is through
    // log4cpp, but not through OCL's log4cpp-derived real-time logging!)
    if (!rttLog4cppConfigFile.empty())
    {
        try
        {
            log4cpp::PropertyConfigurator::configure(rttLog4cppConfigFile);
        }
        catch (log4cpp::ConfigureFailure& e)
        {
            std::cerr << "ERROR: Unable to read/parse log4cpp configuration file\n"
                      << e.what() << std::endl;
            return false;
        }
    }
    else
    {
        // setup default of logging to 'orocos.log' file
        log4cpp::PatternLayout*	layout=0;
        log4cpp::Appender*		appender=0;
        appender = new log4cpp::FileAppender(RTT::Logger::log4cppCategoryName,
                                             "orocos.log");

        layout = new log4cpp::PatternLayout();
        // encode as (ISO date) "yyyymmddTHH:MM:SS.ms category message"
        layout->setConversionPattern("%d{%Y%m%dT%T.%l} [%p] %m%n");
        appender->setLayout(layout);

        log4cpp::Category& category =
            log4cpp::Category::getInstance(RTT::Logger::log4cppCategoryName);
        category.setAppender(appender);
        // appender and layout are now owned by category - do not delete!
    }
    return true;
}

#endif  //  OROSEM_LOG4CPP_LOGGING

// namespace
}
