
#include <rtt/os/main.h>
#include "dev/RelayCardapci2200.hpp"

using namespace RTT;

int ORO_main(int, char**)
{
    RelayCardapci2200 tCard("Test2200");

    tCard.switchOn(0);

    tCard.switchOff(0);

    return 0;
}
