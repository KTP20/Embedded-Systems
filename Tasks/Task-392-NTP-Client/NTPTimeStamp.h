
/*#if !FEATURE_LWIP
    #error [NOT_SUPPORTED] LWIP not supported for this target
#endif
 */
#ifndef __NTPTIMESTAMP__
#define __NTPTIMESTAMP__


#include "uop_msb.h"
using namespace uop_msb;

#include "EthernetInterface.h"
#include "TCPSocket.h"
#include "NTPClient.h"
#include <iostream>
using namespace std;


class NTPTimeStamp 
{
    private:
        time_t AddTimeStamp();
    public:
        NTPTimeStamp();
        time_t GetTimeLCD();
        time_t GetTime();
};

#endif
