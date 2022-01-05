/*
 * Copyight (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "EthernetInterface.h"

// Network interface
EthernetInterface net;
char rbuffer[65];

#define IPV4_HOST_ADDRESS "192.168.137.1"
#define TCP_SOCKET_PORT 8080

#define IPV4_ADDRESS "192.168.137.2"
#define IPV4_MASK "255.255.255.0"
#define IPV4_GATEWAY "192.168.1.1"

DigitalIn BlueButton(USER_BUTTON);

// Socket demo
int main()
{
    // Bring up the ethernet interface
    printf("TEST\n");
    printf("Ethernet socket example\n");
    int i = net.set_network(IPV4_ADDRESS, IPV4_MASK, IPV4_GATEWAY);
    net.connect();
    printf("net connect done, this takes ages\n");
    bool keepGoing = true;
    printf("setting the bool\n");
 
    do {
        // Show the network address
        SocketAddress a;
        printf("making an instance of the class SocketAddress\n");
        int ip = net.get_ip_address(&a);
        printf("IP from net.get_ip_address : %d\n",ip);
        printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

        // Open a socket on the network interface, and create a TCP connection to the TCP server
        TCPSocket socket;
        socket.open(&net);

        //Option 1. Look up IP address of remote machine on the Internet
        //net.gethostbyname("ifconfig.io", &a);
        //printf("IP address of site: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

        //Option 2. Manually set the address (In a Windows terminal, type ipconfig /all and look for the IPV4 Address for the network interface you are using)
        a.set_ip_address(IPV4_HOST_ADDRESS);

        //Set the TCP socket port
        a.set_port(TCP_SOCKET_PORT);

        //Connect to remote web server
        socket.connect(a);

        // Send a simple array of bytes (I've used a string so you can read it)
        char sbuffer[] = "Hello, this is the MBED Board talking!";
        char qbuffer[] = "END";

        int scount;
        if (BlueButton == 0) {
            scount = socket.send(sbuffer, sizeof sbuffer);
        } else {
            printf("Sending END\n");
            scount = socket.send(qbuffer, sizeof qbuffer);
            keepGoing = false;
        }
        
        printf("sent\r\n");

        // ***********************************************************
        // Receive a simple array of bytes as a response and print out
        // ***********************************************************
        printf("Got down to response start\n ");
        int rcount;

        // Receieve response and print out the response line
        while ((rcount = socket.recv(rbuffer, 64)) > 0) {
             printf("Got into the response loop \n");
            rbuffer[rcount] = 0;    //End of string character
            printf("%s", rbuffer);
        }
        printf("got past the response loop\n");

        //Check for error
        if (rcount < 0) {
            printf("Error! socket->recv() returned: %d\n", rcount);
            keepGoing = false;
        }
        printf("past the error check, about to close the socket\n");
        // Close the socket to return its memory and bring down the network interface
        socket.close();

        //Loop delay of 5s
        wait_us(5000000);

    } while (keepGoing);


    // Bring down the ethernet interface
    net.disconnect();
    printf("Disconnected. Now entering sleep mode\n");

    sleep();

}
