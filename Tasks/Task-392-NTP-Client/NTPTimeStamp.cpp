/*#if !FEATURE_LWIP
    #error [NOT_SUPPORTED] LWIP not supported for this target
#endif
 */
#include "uop_msb.h"
using namespace uop_msb;

#include "EthernetInterface.h"
#include "TCPSocket.h"
//#include "NTPClient.h"
#include <iostream>
using namespace std;

#include "NTPTimeStamp.h"

//const char *const myHTTP = HTTP_RESPONSE; //This if you wish to set above Compiler defines into Flash Silicon

NetworkInterface* netIF;
LCD_16X2_DISPLAY disp;
DigitalOut lcdBacklight(LCD_BKL_PIN,1);
InterruptIn blueButton(USER_BUTTON);


NTPTimeStamp::NTPTimeStamp()
{
    NetworkInterface* netIF;
    LCD_16X2_DISPLAY disp;
    DigitalOut lcdBacklight(LCD_BKL_PIN,1);
    InterruptIn blueButton(USER_BUTTON);
  netIF = NetworkInterface::get_default_instance();
    if (netIF == nullptr) {
        printf("No network interface found\n");
    }
    printf("network found\n");   
}

time_t NTPTimeStamp::AddTimeStamp()
{
 int ret = netIF->connect();
    if (ret != 0) {
        printf("Connection error: %d\n", ret);
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
    return timestamp;
}

time_t NTPTimeStamp::GetTimeLCD()
{
    time_t timestamp = AddTimeStamp();
    //Immediately read the time (seconds since 00:00 on the 1/1/1070)
    timestamp = time(NULL);

    //Display the current date and time
    disp.cls(); disp.locate(0, 0);
    disp.printf("%s\n", ctime(&timestamp));
    return timestamp;
}

time_t NTPTimeStamp::GetTime()
{
    time_t timestamp = AddTimeStamp();
    //Immediately read the time (seconds since 00:00 on the 1/1/1070)
    timestamp = time(NULL);

    //Display the current date and time
    //disp.cls(); disp.locate(0, 0);
    printf("%s\n", ctime(&timestamp));
    return timestamp;
}

