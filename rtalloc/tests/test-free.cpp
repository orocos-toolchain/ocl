// Test that real-time allocator and OCL::String correctly free memory

// need access to all TLSF functions embeded in RTT
#define ORO_MEMORY_POOL

#include "rtalloc/String.hpp"


#include <iostream>
#include <vector>
#include <sstream>
#include "funcs.hpp"

// size of pool
static const size_t     POOL_SIZE       = 10*1024;
static char             pool[POOL_SIZE];

#define PRINT_STATS()                                                   \
    std::cout << "TLSF bytes allocated=" << POOL_SIZE                   \
    << " overhead=" << (POOL_SIZE - freeMem)                            \
    << " max-used=" << get_max_size(&pool[0])                           \
    << " currently-used=" << get_used_size(&pool[0])                    \
    << " still-allocated=" << (get_used_size(&pool[0]) - (POOL_SIZE - freeMem))\
    << std::endl

int main()
{
    std::cout << "\nInitialize memory pool\n";
    size_t freeMem = init_memory_pool(sizeof(pool), &pool[0]);
    PRINT_STATS();
    
    std::cout << "\nTest basic allocate/deallocate\n";
    PRINT_STATS();
    void* p = tlsf_malloc(100); // < tlsf::MIN_BLOCK_SIZE
    PRINT_STATS();
    tlsf_free(p);
    p = 0;
    PRINT_STATS();

    p = tlsf_malloc(1024); // > tlsf::MIN_BLOCK_SIZE
    PRINT_STATS();
    tlsf_free(p);
    p = 0;
    PRINT_STATS();
    
    std::cout << "\nTest OCL::String\n";
    {
        OCL::String     s;
        PRINT_STATS();
        s = "Hello World";
        PRINT_STATS();
        s = "";
        PRINT_STATS();
        {
            OCL::String     s2(s);
            PRINT_STATS();
            s2 = "1234567890123456789012345678901234567890";
            PRINT_STATS();
        }
        PRINT_STATS();        
    }
    PRINT_STATS();

    std::cout << "\nTest to exhaustion\n";
    std::vector<OCL::String>    allocatedMem;
    PRINT_STATS();
    for (int i = 0; true; ++i)
    {
        std::cout << i << " ";
        try
        {
            std::stringstream   ss;
            ss << "Hello world " << i;
            OCL::String str = ss.str().c_str();
            allocatedMem.push_back(str);
        }
        catch (std::bad_alloc& e)
        {
            std::cout << "\n";
            break;    
        }
    }
    PRINT_STATS();
    // now free it all
    while (0 < allocatedMem.size())
    {
        allocatedMem.pop_back();
    }
    PRINT_STATS();
        
    std::cout << "\nDestroy memory pool\n";
    destroy_memory_pool(&pool[0]);
    PRINT_STATS();

    return 0;
}
