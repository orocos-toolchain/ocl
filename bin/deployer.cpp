/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:30:14 CEST 2008  deployer.cpp

                        deployer.cpp -  description
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
#include <rtt/Logger.hpp>

#ifndef RTT_SCRIPT_PROGRAM
#define USE_TASKBROWSER
#endif

#ifdef USE_TASKBROWSER
#include <taskbrowser/TaskBrowser.hpp>
#endif
#include <deployment/DeploymentComponent.hpp>
#include <iostream>
#include <string>
#include "deployer-funcs.hpp"

#ifdef  ORO_BUILD_LOGGING
#   ifndef OS_RT_MALLOC
#   warning "Logging needs rtalloc!"
#   endif
#include <log4cpp/HierarchyMaintainer.hh>
#include "logging/Category.hpp"
#endif

using namespace RTT;
namespace po = boost::program_options;
using namespace std;

int main(int argc, char** argv)
{
	std::string                 siteFile;      // "" means use default in DeploymentComponent.cpp
    std::vector<std::string>    scriptFiles;
	std::string                 name("Deployer");
    bool                        requireNameService = false;         // not used
    bool                        deploymentOnlyChecked = false;
	int							minNumberCPU = 0;
    po::variables_map           vm;
	po::options_description     otherOptions;

#ifdef  ORO_BUILD_RTALLOC
    OCL::memorySize             rtallocMemorySize   = ORO_DEFAULT_RTALLOC_SIZE;
	po::options_description     rtallocOptions      = OCL::deployerRtallocOptions(rtallocMemorySize);
	otherOptions.add(rtallocOptions);
#endif

#if     defined(ORO_BUILD_LOGGING) && defined(OROSEM_LOG4CPP_LOGGING)
    // to support RTT's logging to log4cpp
    std::string                 rttLog4cppConfigFile;
    po::options_description     rttLog4cppOptions = OCL::deployerRttLog4cppOptions(rttLog4cppConfigFile);
    otherOptions.add(rttLog4cppOptions);
#endif

    // were we given non-deployer options? ie find "--"
    int     optIndex    = 0;
    bool    found       = false;

    while(!found && optIndex<argc)
    {
        found = (0 == strcmp("--", argv[optIndex]));
        if(!found) optIndex++;
    }

    if (found) {
        argv[optIndex] = argv[0];
    }

    // if extra options not found then process all command line options,
    // otherwise process all options up to but not including "--"
    int rc = OCL::deployerParseCmdLine(!found ? argc : optIndex, argv,
                                       siteFile, scriptFiles, name, requireNameService,deploymentOnlyChecked,
									   minNumberCPU,
                                       vm, &otherOptions);

    if (0 != rc)
	{
		return rc;
	}

	// check system capabilities
	rc = OCL::enforceMinNumberCPU(minNumberCPU);
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
    size_t                  freeMem     = 0;
    if (0 < memSize)
    {
        // don't calloc() as is first thing TLSF does.
        rtMem = malloc(memSize);
        assert(0 != rtMem);
        freeMem = init_memory_pool(memSize, rtMem);
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
    log4cpp::HierarchyMaintainer::set_category_factory(
        OCL::logging::Category::createOCLCategory);
#endif


    /******************** WARNING ***********************
     *   NO log(...) statements before __os_init() !!!!! 
     ***************************************************/

    // start Orocos _AFTER_ setting up log4cpp
	if (0 == __os_init(argc - optIndex, &argv[optIndex]))
    {
#ifdef  ORO_BUILD_LOGGING
        log(Info) << "OCL factory set for real-time logging" << endlog();
#endif
        // scope to force dc destruction prior to memory free
        {
            OCL::DeploymentComponent dc( name, siteFile );

            /* Only start the scripts after the Orb was created. Processing of
               scripts stops after the first failed script, and -1 is returned.
               Whether a script failed or all scripts succeeded, in non-daemon
               and non-checking mode the TaskBrowser will be run to allow
               inspection.
             */
            bool result = true;
            for (std::vector<std::string>::const_iterator iter=scriptFiles.begin();
                 iter!=scriptFiles.end() && result;
                 ++iter)
            {
                if ( !(*iter).empty() )
                {
                    if ( (*iter).rfind(".xml",std::string::npos) == (*iter).length() - 4 || (*iter).rfind(".cpf",std::string::npos) == (*iter).length() - 4) {
                        if ( deploymentOnlyChecked ) {
                            if (!dc.loadComponents( (*iter) )) {
                                result = false;
                                log(Error) << "Failed to load file: '"<< (*iter) <<"'." << endlog();
                            } else if (!dc.configureComponents()) {
                                result = false;
                                log(Error) << "Failed to configure file: '"<< (*iter) <<"'." << endlog();
                            }
                            // else leave result=true and continue
                        } else {
                            result = dc.kickStart( (*iter) );
                        }
                        continue;
                    }

                    if ( (*iter).rfind(".ops",std::string::npos) == (*iter).length() - 4 ||
                         (*iter).rfind(".osd",std::string::npos) == (*iter).length() - 4 ||
                         (*iter).rfind(".lua",std::string::npos) == (*iter).length() - 4) {
                        result = dc.runScript( (*iter) ) && result;
                        continue;
                    }
                    log(Error) << "Unknown extension of file: '"<< (*iter) <<"'. Must be xml, cpf for XML files or, ops, osd or lua for script files."<<endlog();
                }
            }
            rc = (result ? 0 : -1);
#ifdef USE_TASKBROWSER
            // We don't start an interactive console when we're a daemon
            if ( !deploymentOnlyChecked && !vm.count("daemon") ) {
                OCL::TaskBrowser tb( &dc );

                tb.loop();

                dc.shutdownDeployment();
            }
#endif
        }
#ifdef  ORO_BUILD_RTALLOC
        if (0 != rtMem)
            {
                // print statistics after deployment finished, but before os_exit() (needs Logger):
                log(Debug) << "TLSF bytes allocated=" << memSize
                           << " overhead=" << (memSize - freeMem)
                           << " max-used=" << get_max_size(rtMem)
                           << " currently-used=" << get_used_size(rtMem)
                           << " still-allocated=" << (get_used_size(rtMem) - (memSize - freeMem))
                           << endlog();
            }
#endif

        __os_exit();
	}
	else
	{
		std::cerr << "Unable to start Orocos" << std::endl;
        rc = -1;
	}

#ifdef  ORO_BUILD_LOGGING
    log4cpp::HierarchyMaintainer::getDefaultMaintainer().shutdown();
    log4cpp::HierarchyMaintainer::getDefaultMaintainer().deleteAllCategories();
#endif

#ifdef  ORO_BUILD_RTALLOC
    if (0 != rtMem)
    {
        std::cout << "TLSF bytes allocated=" << memSize
                  << " overhead=" << (memSize - freeMem)
                  << " max-used=" << get_max_size(rtMem)
                  << " currently-used=" << get_used_size(rtMem)
                  << " still-allocated=" << (get_used_size(rtMem) - (memSize - freeMem))
                  << "\n";

        destroy_memory_pool(rtMem);
        free(rtMem);
    }
#endif

    return rc;
}
