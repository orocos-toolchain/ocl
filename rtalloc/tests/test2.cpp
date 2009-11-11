// Test behaviour on exhaustion of of real-time allocated memory

// need access to all TLSF functions embeded in RTT
#define ORO_MEMORY_POOL
#include <rtt/os/tlsf/tlsf.h>

#include <iostream>
#include <vector>
#include "funcs.hpp"

// whether to print debug data (this does cause page faults)
bool                    debug           = false;

// size of pool
static const size_t     POOL_SIZE       = 20*1024;
// size of allocation used to exhaust pool
static const size_t     ALLOC_SIZE      = 1024;
static char             pool[POOL_SIZE];

// page fault data
static struct rusage    initial;

int main()
{
    setAsRealtime();    
    memset(&pool[0], 0, sizeof(pool));
    
    // avoid page faults by reserving array up front
    std::vector<void*>  allocatedMem;
    allocatedMem.reserve(2 * (POOL_SIZE / ALLOC_SIZE));
    
    size_t freeMem = init_memory_pool(sizeof(pool), &pool[0]);
    std::cout << freeMem << " bytes free of " << POOL_SIZE << " bytes total." << std::endl;
    outputTLSFStatistics(&pool[0]);

    // after the memory pool init and first std::cout, as they generate faults
    initializePageFaults(initial);
    outputPageFaults(initial);

    // exhaust memory
    std::cout << "Exhausting memory with allocations of " << ALLOC_SIZE << " bytes." << std::endl;
    outputPageFaults(initial);
    for (int i = 0; true; ++i)
    {
        if (debug) std::cout << i;
        void* p = tlsf_malloc(ALLOC_SIZE);
        {
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            if ((0 != usage.ru_majflt - initial.ru_majflt) ||
                (0 != usage.ru_minflt - initial.ru_minflt))
            {
                std::cout << "Page fault on allocation " << i << "\n";
            }
        }

        if (p)
        {
            allocatedMem.push_back(p);
        }
        else
        {
            if (debug) std::cout << "-FAILED\n";
            break;    
        }
        if (debug) std::cout << " ";
    }
    outputPageFaults(initial);

    // now free it all and see what we've got left
    std::cout << "Freeing memory" << std::endl;
    for (int i=0; 0 < allocatedMem.size(); ++i)
    {
        if (debug) std::cout << i << " ";
        tlsf_free(allocatedMem.back());
        allocatedMem.pop_back();
    }
    if (debug) std::cout << std::endl;
    outputPageFaults(initial);

    outputTLSFStatistics(&pool[0]);

    return 0;
}
