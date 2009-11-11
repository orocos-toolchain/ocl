// Test basic use of real-time allocator

// need access to all TLSF functions embeded in RTT
#define ORO_MEMORY_POOL
#include "rtalloc/String.hpp"

#include <iostream>
#include "funcs.hpp"

// size of pool
static const size_t     POOL_SIZE       = 20*1024;
static char             pool[POOL_SIZE];

// page fault data
static struct rusage    initial;

int main()
{
    size_t freeMem = init_memory_pool(sizeof(pool), &pool[0]);
    std::cout << freeMem << " bytes free of " << POOL_SIZE << " bytes total." << std::endl;
    
    // after the memory pool init and first std::cout, as they generate faults
    initializePageFaults(initial);
    outputPageFaults(initial);

    OCL::String     s;
    s = "Hello World";
    outputPageFaults(initial);

    std::cout << s << std::endl;

    return 0;
}
