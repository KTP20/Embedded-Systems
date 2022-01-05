/*#if !FEATURE_LWIP
    #error [NOT_SUPPORTED] LWIP not supported for this target
#endif
 */
#include "uop_msb.h"
using namespace uop_msb;

#include "EthernetInterface.h"
#include "TCPSocket.h"
//#include "NTPClient.h"
#include "NTPTimeStamp.h"
#include <iostream>
using namespace std;

//const char *const myHTTP = HTTP_RESPONSE; //This if you wish to set above Compiler defines into Flash Silicon


int main()
{
    NTPTimeStamp();
    GetTimeLCD();

}


/*
int main()
{
    printf("start\n");
    // Create network interface (Ethernet in our case)    
    netIF = NetworkInterface::get_default_instance();
    if (netIF == nullptr) {
        printf("No network interface found\n");
        return -1;
    }
    printf("network found\n");
    // Connect
    //netIF->set_network("192.168.0.2", "255.255.255.0", "192.168.1.1");  //For static IP
    int ret = netIF->connect();
    if (ret != 0) {
        printf("Connection error: %d\n", ret);
        return -1;
    }
    printf("Connection success, MAC: %s\n", netIF->get_mac_address());
    printf("Getting time from the NTP server\n");

    // Instantiate a NPTClient using the connected interface
    NTPClient ntp(netIF);
    printf("NPTClient initiatied\n");
    // Configure for a public time server
    ntp.set_server("time.google.com", 123);


    // Get the time
    time_t timestamp = ntp.get_timestamp();
    if (timestamp < 0) {
        cout << "Failed to get the current time, error: " << timestamp << endl;
        netIF->disconnect(); //This is easy to forget!
        return -1;
    }
    printf("Time: %s\n", ctime(&timestamp));
    // Set system time
    set_time(timestamp);
    
    //Close network connection (we are finished)
    printf("Closing network connection\n");
    netIF->disconnect();

    while (true)
    {
        //Spin waiting for button press
        while (blueButton == 0);

        //Immediately read the time (seconds since 00:00 on the 1/1/1070)
        timestamp = time(NULL);

        //Display the current date and time
        disp.cls(); disp.locate(0, 0);
        disp.printf("%s\n", ctime(&timestamp));
        
        //Debounce
        ThisThread::sleep_for(250ms);
        
        //Spin waiting for button release
        while (blueButton == 1);

        //Debounce
        ThisThread::sleep_for(250ms);
    }


}
 
*/