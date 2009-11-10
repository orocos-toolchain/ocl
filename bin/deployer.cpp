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

#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <rtt/Logger.hpp>

#ifdef ORO_BUILD_RTALLOC
// need access to all TLSF functions embedded in RTT
#define ORO_MEMORY_POOL
#include <rtt/os/tlsf/tlsf.h>
#endif

#include <taskbrowser/TaskBrowser.hpp>
#include <deployment/DeploymentComponent.hpp>
#include <iostream>
#include <string>
#include "deployer-funcs.hpp"

#ifdef  ORO_BUILD_LOGGING
#   ifndef ORO_BUILD_RTALLOC
#   warning Logging needs rtalloc!
#   endif
#include <log4cpp/HierarchyMaintainer.hh>
#include "logging/Category.hpp"
#endif

using namespace RTT;
namespace po = boost::program_options;

#ifdef  TEST_RTALLOC
int main(int argc, char** argv)
#else
int ORO_main(int argc, char** argv)
#endif
{
	std::string             siteFile;      // "" means use default in DeploymentComponent.cpp
	std::string             scriptFile;
	std::string             name("Deployer");
    bool                    requireNameService = false;         // not used
    po::variables_map       vm;
	po::options_description* otherOptions    = 0;

#ifdef  ORO_BUILD_RTALLOC
    OCL::memorySize         rtallocMemorySize   = ORO_DEFAULT_RTALLOC_SIZE;
	po::options_description rtallocOptions      = OCL::deployerRtallocOptions(rtallocMemorySize);
	otherOptions                                = &rtallocOptions;
#endif

	int rc = OCL::deployerParseCmdLine(argc, argv,
                                       siteFile, scriptFile, name, requireNameService,
                                       vm, otherOptions);
	if (0 != rc)
	{
		return rc;
	}

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
            log(Critical) << "Invalid memory pool size of " << memSize 
                          << " bytes (TLSF has a several kilobyte overhead)." << endlog();
            free(rtMem);
            return -1;
        }
        log(Info) << "Real-time memory: " << freeMem << " bytes free of "
                  << memSize << " allocated." << endlog();
    }
#endif  // ORO_BUILD_RTALLOC

#ifdef  ORO_BUILD_LOGGING
    log(Info) << "Setting OCL factory for real-time logging" << endlog();
    log4cpp::HierarchyMaintainer::set_category_factory(
        OCL::logging::Category::createOCLCategory);
#endif

    // scope to force dc destruction prior to memory free
    {
        OCL::DeploymentComponent dc( name, siteFile );

        if ( !scriptFile.empty() )
        {
            dc.kickStart( scriptFile );
        }

        OCL::TaskBrowser tb( &dc );

        tb.loop();
#ifdef  ORO_BUILD_RTALLOC
        if (0 != rtMem)
        {
            // warning: at this point all RTT components haven't yet been
            // stopped and cleaned up, so any RT memory there hasn't yet been
            // deallocated!
            log(Debug) << "TLSF bytes allocated=" << memSize
                       << " overhead=" << (memSize - freeMem)
                       << " max-used=" << get_max_size(rtMem)
                       << " currently-used=" << get_used_size(rtMem)
                       << " still-allocated=" << (get_used_size(rtMem) - (memSize - freeMem))
                       << endlog();
        }
#endif
    }

#ifdef  TEST_RTALLOC
    __os_exit();

#ifdef  ORO_BUILD_LOGGING
    log(Debug) << "Deleting all logging categories" << endlog();
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
    }
#endif
#endif

#ifdef  ORO_BUILD_RTALLOC
    if (0 != rtMem)
    {
        destroy_memory_pool(rtMem);
        free(rtMem);
    }
#endif

    return 0;
}
