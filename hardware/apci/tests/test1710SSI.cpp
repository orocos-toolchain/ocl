
#include <rtt/os/main.h>
#include "dev/EncoderSSIapci1710.hpp"

using namespace RTT;

int ORO_main(int, char**)
{
    // use first modules
    EncoderSSI_apci1710_board tCard(0);

    // read 3 encoders of first module.
    tCard.read(0);
    tCard.read(1);
    tCard.read(2);

    return 0;
}
