
#include <rtt/os/main.h>
#include "dev/SwitchDigitalInapci1032.hpp"

using namespace RTT;

int ORO_main(int, char**)
{
    SwitchDigitalInapci1032 tCard("Test1032");

    tCard.isOn(0);

    tCard.isOff(0);

    return 0;
}
