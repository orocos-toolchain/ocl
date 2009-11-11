#ifndef __FUNCS_HPP
#define __FUNCS_HPP 1

#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <iostream>

#ifdef  __linux__
#include <sys/time.h>		// needed for getrusage
#include <sys/resource.h>	// needed for getrusage
#endif

// output TLSF statistics to std::cout
inline void outputTLSFStatistics(void* memPool)
{
    // this only works if you compile TLSF with statistics on, else you get 0
    std::cout << "TLSF max size=" << get_max_size(memPool) << " bytes\n";
    std::cout << "TLSF used size=" << get_used_size(memPool) << " bytes\n";
}

inline void initializePageFaults(struct rusage& initial)
{
#ifdef  __linux__
    std::cout << "## Page faults initialized" << std::endl;
    getrusage(RUSAGE_SELF, &initial);
#else
    memset(&initial, 0, sizeof(initial));   // avoid compiler warning
#endif
}

inline void outputPageFaults(const struct rusage& initial)
{
#ifdef  __linux__
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    std::cout << "Faults major " << usage.ru_majflt - initial.ru_majflt
              << " minor " << usage.ru_minflt - initial.ru_minflt << std::endl;
#else
    // do nothing
#endif
}

/* take real-time precautions:
   - lock all memory pages
   - write to all data we want to deal with in real-time (ie no page faults)
*/
inline void setAsRealtime()
{
#ifdef  __linux__
    struct sched_param param;
    param.sched_priority = 1;       // value doesn't really matter
    if (-1 == sched_setscheduler(0, SCHED_FIFO, &param))
    {
        perror("sched_setscheduler failed");
        exit(-1);
    }
    if (-1 == mlockall(MCL_CURRENT|MCL_FUTURE))
    {
        perror("mlockall failed");
        exit(-2);
    }
    // really should write to our entire stack also ...
#else
// do nothing
#endif
}

#endif
