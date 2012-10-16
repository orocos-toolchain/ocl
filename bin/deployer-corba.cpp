/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:30:24 CEST 2008  deployer-corba.cpp

                        deployer-corba.cpp -  description
                           -------------------
    begin                : Thu July 03 2008
    copyright            : (C) 2008 Peter Soetens
    email                : peter.soetens@fmtc.be

 ***************************************************************************
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 ***************************************************************************/


#include <rtt/rtt-config.h>
#ifdef OS_RT_MALLOC
// need access to all TLSF functions embedded in RTT
#define ORO_MEMORY_POOL
#include <rtt/os/tlsf/tlsf.h>
#endif
#include <rtt/os/main.h>
#include <rtt/RTT.hpp>

#include <taskbrowser/TaskBrowser.hpp>
#include <deployment/CorbaDeploymentComponent.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <iostream>
#include "deployer-funcs.hpp"

#include <rtt/transports/corba/corba.h>

#ifdef  ORO_BUILD_LOGGING
#   ifndef OS_RT_MALLOC
#   warning "Logging needs rtalloc!"
#   endif
#include <log4cpp/HierarchyMaintainer.hh>
#include "logging/Category.hpp"
#endif

using namespace std;
using namespace RTT;
using namespace RTT::corba;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
	std::string                 siteFile;      // "" means use default in DeploymentComponent.cpp
    std::vector<std::string>    scriptFiles;
	std::string                 name("Deployer");
    bool                        requireNameService = false;
	po::variables_map           vm;
	po::options_description     taoOptions("Additional options can also be passed to TAO");
	// we don't actually list any options for TAO ...

	po::options_description     otherOptions;

#ifdef  ORO_BUILD_RTALLOC
    OCL::memorySize         rtallocMemorySize   = ORO_DEFAULT_RTALLOC_SIZE;
	po::options_description rtallocOptions      = OCL::deployerRtallocOptions(rtallocMemorySize);
	otherOptions.add(rtallocOptions);
#endif

#if     defined(ORO_BUILD_LOGGING) && defined(OROSEM_LOG4CPP_LOGGING)
    // to support RTT's logging to log4cpp
    std::string                 rttLog4cppConfigFile;
    po::options_description     rttLog4cppOptions = OCL::deployerRttLog4cppOptions(rttLog4cppConfigFile);
    otherOptions.add(rttLog4cppOptions);
#endif

    // as last set of options
    otherOptions.add(taoOptions);

    // were we given TAO options? ie find "--"
    int     taoIndex    = 0;
    bool    found       = false;

    while(!found && taoIndex<argc)
    {
        found = (0 == strcmp("--", argv[taoIndex]));
        if(!found) taoIndex++;
    }

    if (found) {
        argv[taoIndex] = argv[0];
    }

    // if TAO options not found then process all command line options,
    // otherwise process all options up to but not including "--"
	int rc = OCL::deployerParseCmdLine(!found ? argc : taoIndex, argv,
                                       siteFile, scriptFiles, name, requireNameService,
                                       vm, &otherOptions);
	if (0 != rc)
	{
		return rc;
	}


#if     defined(ORO_BUILD_LOGGING) && defined(OROSEM_LOG4CPP_LOGGING)
    if (!OCL::deployerConfigureRttLog4cppCategory(rttLog4cppConfigFile))
    {
        return -1;
    }
#endif

#ifdef  ORO_BUILD_RTALLOC
    size_t                  memSize     = rtallocMemorySize.size;
    void*                   rtMem       = 0;
    if (0 < memSize)
    {
        // don't calloc() as is first thing TLSF does.
        rtMem = malloc(memSize);
        assert(rtMem);
        size_t freeMem = init_memory_pool(memSize, rtMem);
        if ((size_t)-1 == freeMem)
        {
            cerr << "Invalid memory pool size of " << memSize
                          << " bytes (TLSF has a several kilobyte overhead)." << endl;
            free(rtMem);
            return -1;
        }
        cout << "Real-time memory: " << freeMem << " bytes free of "
                  << memSize << " allocated." << endl;
    }
#endif  // ORO_BUILD_RTALLOC

#ifdef  ORO_BUILD_LOGGING
    // use our log4cpp-derived categories to do real-time logging
    log4cpp::HierarchyMaintainer::set_category_factory(
        OCL::logging::Category::createOCLCategory);
#endif

    // start Orocos _AFTER_ setting up log4cpp
	if (0 == __os_init(argc - taoIndex, &argv[taoIndex]))
    {
#ifdef  ORO_BUILD_LOGGING
        log(Info) << "OCL factory set for real-time logging" << endlog();
#endif
        rc = -1;     // prove otherwise
        try {
            // if TAO options not found then have TAO process just the program name,
            // otherwise TAO processes the program name plus all options (potentially
            // none) after "--"
            TaskContextServer::InitOrb( argc - taoIndex, &argv[taoIndex] );

            OCL::CorbaDeploymentComponent dc( name, siteFile );

            if (0 == TaskContextServer::Create( &dc, true, requireNameService ))
                {
                    return -1;
                }

            // The orb thread accepts incomming CORBA calls.
            TaskContextServer::ThreadOrb();

            // Only start the scripts after the Orb was created.
            for (std::vector<std::string>::const_iterator iter=scriptFiles.begin();
                 iter!=scriptFiles.end();
                 ++iter)
            {
                if ( !(*iter).empty() )
                {
                    if ( (*iter).rfind(".xml",string::npos) == (*iter).length() - 4 || (*iter).rfind(".cpf",string::npos) == (*iter).length() - 4) {
                        dc.kickStart( (*iter) );
                        continue;
                    } if ( (*iter).rfind(".ops",string::npos) == (*iter).length() - 4 || (*iter).rfind(".osd",string::npos) == (*iter).length() - 4) {
                        dc.runScript( (*iter) );
                        continue;
                    }
                    log(Error) << "Unknown extension of file: '"<< (*iter) <<"'. Must be xml, cpf for XML files or, ops or osd for script files."<<endlog();
                }
            }

            if ( !vm.count("daemon") ) {
                 OCL::TaskBrowser tb( &dc );
                 tb.loop();

                 // do it while CORBA is still up in case need to do anything remote.
                 dc.shutdownDeployment();
            }

            TaskContextServer::ShutdownOrb();

            TaskContextServer::DestroyOrb();

            rc = 0;

        } catch( CORBA::Exception &e ) {
            log(Error) << argv[0] <<" ORO_main : CORBA exception raised!" << Logger::nl;
            log() << CORBA_EXCEPTION_INFO(e) << endlog();
        } catch (...) {
            // catch this so that we can destroy the TLSF memory correctly
            log(Error) << "Uncaught exception." << endlog();
        }

		// shutdown Orocos
		__os_exit();
	}
	else
	{
		std::cerr << "Unable to start Orocos" << std::endl;
        rc = -1;
	}

#ifdef  ORO_BUILD_RTALLOC
    if (!rtMem)
    {
        destroy_memory_pool(rtMem);
        free(rtMem);
    }
#endif  // ORO_BUILD_RTALLOC

    return rc;
}
