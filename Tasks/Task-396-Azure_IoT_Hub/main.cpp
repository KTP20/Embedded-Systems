/*
 * Copyright (c) 2020 Arm Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "rtos/ThisThread.h"
#include "NTPClient.h"

#include "certs.h"
#include "iothub.h"
#include "iothub_client_options.h"
#include "iothub_device_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/xlogging.h"

#include "iothubtransportmqtt.h"
#include "azure_cloud_credentials.h"
#include <cstdio>
#include <cstring>
#include <string.h>

#include "uop_msb.h"
/**
 * This example sends and receives messages to and from Azure IoT Hub.
 * The API usages are based on Azure SDK's official iothub_convenience_sample.
 */

// Global symbol referenced by the Azure SDK's port for Mbed OS, via "extern"


NetworkInterface *_defaultSystemNetwork;

static bool message_received = false;
int temp[] ={0,0};
int light[] = {0,0};
int pressure[] = {0,0};
char RESPONSE_STRING[] = "Failue to send command";

static void on_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) {
        LogInfo("Connected to IoT Hub");
    } else {
        LogError("Connection failed, reason: %s", MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason));
    }
}

// **************************************
// * MESSAGE HANDLER (no response sent) *
// **************************************
DigitalOut led2(LED2);
static IOTHUBMESSAGE_DISPOSITION_RESULT on_message_received(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
    LogInfo("Message received from IoT Hub");

    const unsigned char *data_ptr;
    size_t len;
    if (IoTHubMessage_GetByteArray(message, &data_ptr, &len) != IOTHUB_MESSAGE_OK) {
        LogError("Failed to extract message data, please try again on IoT Hub");
        return IOTHUBMESSAGE_ABANDONED;
    }

    message_received = true;
    LogInfo("Message body: %.*s", len, data_ptr);

    if (strncmp("true", (const char*)data_ptr, len) == 0) {
        led2 = 1;
    } else {
        led2 = 0;
    }

    return IOTHUBMESSAGE_ACCEPTED;
}

static void on_message_sent(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
        LogInfo("Message sent successfully");
    } else {
        LogInfo("Failed to send message, error: %s",
            MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    }
}

// ****************************************************
// * COMMAND HANDLER (sends a response back to Azure) *
// ****************************************************
DigitalOut led1(LED1);
DigitalOut led3(LED3);
DigitalIn blueButton(USER_BUTTON);
uop_msb::LCD_16X2_DISPLAY lcd;

void LcdValue(const char* payload)
{
    lcd.cls();
    lcd.printf("value: %s",(const char*)payload);
}

void RedLEDToggle(const char* payload, size_t size)
{
    if (strncmp("true", (const char*)payload, size) == 0 ) {
        printf("LED3 ON\n");
        led3 = 1;
    } else {
        printf("LED3 OFF\n");
        led3 = 0;
    }
}

void GreenLEDToggle(const char* payload, size_t size)
{
    if ( strncmp("true", (const char*)payload, size) == 0 ) {
        printf("LED1 ON\n");
        led1 = 1;
    } else {
        printf("LED1 OFF\n");
        led1 = 0;
    }
}

int SetThreshold(const char* payload, int size, int* temp, int* light, int* pressure)
{
    printf("entered SetLow\n");
    int j = 0;
    int l = 0;
    char xtemp[10];
    char ytemp[10];
    char ztemp[10];

    for (int i=0;i<2;i++)
    {
        int startOfPhrase = j;
        while(payload[l] != ':')
        {
            l++;
        }
        while(payload[j] != ','){
            j++;
            if(j>size){
                break;
            } 
        }
        int counter = 0;
        for(int k=l+1;k<j;k++)
        { 
            switch(i){
            case 0:
                xtemp[counter] = payload[k];
            break;
            case 1:
                ytemp[counter] = payload[k];
            break;
            case 2:
                ztemp[counter] = payload[k];
            break;
            default:
                printf("Error");
            break; 
            }
            counter++;
        }
        j++;
        l++;
    }
    while(payload[l] != ':')
    {
        l++;
    }
    while(payload[j] != '}'){
        j++;
        if(j>size){
            break;
        } 
    }
    int counter = 0;
    for(int k=l+1;k<j;k++)
        {
            ztemp[counter] = payload[k];
            counter++;
        }
    int x = std::atoi(xtemp);
    int y = std::atoi(ytemp);
    int z = std::atoi(ztemp);
    

    printf("x = %d, y = %d, z = %d\n",x,y,z);
    *temp = x;
    *light = y;
    *pressure = z;

    return 1;
} 

void Plot(const char* payload, int size, bool* success)
{
   
    bool Recieved =false;
        if(strncmp("\"T\"", (const char*)payload, size) == 0 ){
            printf("Temperature output");
            //assign Temperature to the LED Matrix
            //maybe a message or a function call
            Recieved = true;
        }
        else if(strncmp("\"P\"", (const char*)payload, size) == 0 ){
            printf("Pressure Output");
            //same applies
            //TBC
            Recieved = true;
        }
        else if(strncmp("\"L\"", (const char*)payload, size) == 0 ){
            printf("Light output");
            Recieved = true;
        } 
        else{
            printf("Error in the Plot function, invalid Character");
            Recieved = false;
        }
    *success = Recieved;
}
static int on_method_callback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback)
{
    const char* device_id = (const char*)userContextCallback;
    const char * input_string = (const char *) payload;
    char RESPONSE_STRING[] = "\"Failue to send command\"";
    bool messageSent = 1;
    printf("\r\nDevice Method called for device %s\r\n", device_id);
    printf("Device Method name:    %s\r\n", method_name);
    printf("Device Method payload: %.*s\r\n", (int)size, input_string);

    if (strncmp("ToggleLED",(const char*)method_name, 11) == 0) {
        GreenLEDToggle((const char*)payload, (size_t)size);
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");
    }
    else if (strncmp("ToggleLED3",(const char*)method_name, 11) == 0) {
        RedLEDToggle(input_string,size);
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");
    } 
    else if(strncmp("TestValue",(const char*)method_name, 11) == 0) {
        LcdValue(input_string);
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");
    }
    else if(strncmp("SetLowVector",(const char*)method_name, 12) == 0){
        SetThreshold(input_string,size,&(temp[0]),&(light[0]),&(pressure[0]));
         printf("low threshold: temp is %d, light is %d, pressure is %d",temp[0], light[0],pressure[0]);
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");
    } 
    else if(strncmp("SetHighVector",(const char*)method_name, 13) == 0){
        SetThreshold(input_string,size,&(temp[1]),&(light[1]),&(pressure[1]));
        printf("highthreshold: temp is %d, light is %d, pressure is %d\n",temp[1], light[1],pressure[1]);
        strcpy(RESPONSE_STRING, "{ \"Response\" : \"Command Recieved\" }");
    } 
    else if (strncmp("Plot",(const char*)method_name, 13) == 0){
        bool success = false;
        printf("PLOT payload is %s\t size is %u\n",payload,size);
        Plot(input_string,size,&success);
        if (success == true){
            strcpy(RESPONSE_STRING, "{ \"Response\" :\"Commmand Recieved\"");
        } else {
            strcpy(RESPONSE_STRING, "{ \"Response\" :\"Invalid Character for the plot function, please choose a T,P or L\"");
        }
    }
    else {
        printf("error somewhere lol");
        strcpy(RESPONSE_STRING, "{ \"Responce\" : \"Failue to send command\" }");
    }

    int status = 200;
   
    printf("\r\nResponse status: %d\r\n", status);
    printf("Response payload: %s\r\n\r\n", RESPONSE_STRING);

    int rlen = strlen(RESPONSE_STRING);
    *response_size = rlen;
    if ((*response = (unsigned char*)malloc(rlen)) == NULL) {
        status = -1;
    }
    else {
        memcpy(*response, RESPONSE_STRING, *response_size);
    }
    return status;
}




void demo() {
    bool trace_on = MBED_CONF_APP_IOTHUB_CLIENT_TRACE;
    tickcounter_ms_t interval = 100;
    IOTHUB_CLIENT_RESULT res;

    LogInfo("Initializing IoT Hub client");
    IoTHub_Init();

    IOTHUB_DEVICE_CLIENT_HANDLE client_handle = IoTHubDeviceClient_CreateFromConnectionString(
        azure_cloud::credentials::iothub_connection_string,
        MQTT_Protocol
    );
    if (client_handle == nullptr) {
        LogError("Failed to create IoT Hub client handle");
        goto cleanup;
    }

    // Enable SDK tracing
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_LOG_TRACE, &trace_on);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to enable IoT Hub client tracing, error: %d", res);
        goto cleanup;
    }

    // Enable static CA Certificates defined in the SDK
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_TRUSTED_CERT, certificates);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set trusted certificates, error: %d", res);
        goto cleanup;
    }

    // Process communication every 100ms
    res = IoTHubDeviceClient_SetOption(client_handle, OPTION_DO_WORK_FREQUENCY_IN_MS, &interval);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set communication process frequency, error: %d", res);
        goto cleanup;
    }

    // set incoming message callback
    res = IoTHubDeviceClient_SetMessageCallback(client_handle, on_message_received, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set message callback, error: %d", res);
        goto cleanup;
    }

    // Set incoming command callback
    res = IoTHubDeviceClient_SetDeviceMethodCallback(client_handle, on_method_callback, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set method callback, error: %d", res);
        goto cleanup;
    }

    // Set connection/disconnection callback
    res = IoTHubDeviceClient_SetConnectionStatusCallback(client_handle, on_connection_status, nullptr);
    if (res != IOTHUB_CLIENT_OK) {
        LogError("Failed to set connection status callback, error: %d", res);
        goto cleanup;
    }

    // Send ten message to the cloud (one per second)
    // or until we receive a message from the cloud
    IOTHUB_MESSAGE_HANDLE message_handle;
    char message[80];
    for (int i = 0; i < 10; ++i) {
        if (message_received) {
            // If we have received a message from the cloud, don't send more messeges
            break;
        }
        //Send data in this format:
        /*
            {
                "LightLevel" : 0.12,
                "Temperature" : 36.0,
                "Pressure"  : 3.5
            }

        */
        double light = (float) i;
        double temp  = (float)36.0f-0.1*(float)i;
        double pressure = (float) i+2;
        sprintf(message, "{ \"LightLevel\" : %5.2f, \"Temperature\" : %5.2f, \"Pressure\" : %5.2f}", light, temp, pressure);
        LogInfo("Sending: \"%s\"", message);

        message_handle = IoTHubMessage_CreateFromString(message);
        if (message_handle == nullptr) {
            LogError("Failed to create message");
            goto cleanup;
        }

        res = IoTHubDeviceClient_SendEventAsync(client_handle, message_handle, on_message_sent, nullptr);
        IoTHubMessage_Destroy(message_handle); // message already copied into the SDK

        if (res != IOTHUB_CLIENT_OK) {
            LogError("Failed to send message event, error: %d", res);
            goto cleanup;
        }

        ThisThread::sleep_for(60s);
    }

    // If the user didn't manage to send a cloud-to-device message earlier,
    // let's wait until we receive one
    while (!message_received) {
        // Continue to receive messages in the communication thread
        // which is internally created and maintained by the Azure SDK.
        sleep();
    }

cleanup:
    IoTHubDeviceClient_Destroy(client_handle);
    IoTHub_Deinit();
}

int main() {
    printf("Stating, print \n");
    LogInfo("Connecting to the network");

    _defaultSystemNetwork = NetworkInterface::get_default_instance();
    if (_defaultSystemNetwork == nullptr) {
        LogError("No network interface found");
        return -1;
    }

    int ret = _defaultSystemNetwork->connect();
    if (ret != 0) {
        LogError("Connection error: %d", ret);
        return -1;
    }
    LogInfo("Connection success, MAC: %s", _defaultSystemNetwork->get_mac_address());

    LogInfo("Getting time from the NTP server");

    NTPClient ntp(_defaultSystemNetwork);
    ntp.set_server("time.google.com", 123);
    time_t timestamp = ntp.get_timestamp();
    if (timestamp < 0) {
        LogError("Failed to get the current time, error: %u", timestamp);
        return -1;
    }
    LogInfo("Time: %s", ctime(&timestamp));
    printf("Testing printing\n");
    LogInfo("Testing Logging\n");
    set_time(timestamp);

    LogInfo("Starting the Demo");
    demo();
    LogInfo("The demo has ended");

    return 0;
}
