#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "tcpClientSocket.h"

int main (int argc, char **argv)
{

    //Values passed into client from command line call. 
    //Not read from config file, that will be implemented later. 
    std::string hostname = "";
    std::string username = "";
    int serverport = 0;
    std::string configFile = "";
    std::string testFile = "";
    std::string logFile = "";

    opterr = 0;
    char c = ' ';
    while ((c = getopt (argc, argv, "h:u:p:c:t:L:")) != -1)
        switch (c)
        {
        case 'h':
            hostname = optarg;
            break;
        case 'u':
            username = optarg;
            break;
        case 'p':
            serverport = atoi(optarg);
            if(serverport == 0){
                std::cerr << "Incorrect port number. Please enter an integer\n";
                return 1;
            }
            break;
        case 'c':
            configFile = optarg;
            break;
        case 't':
            //if this is set, we run the test
            testFile = optarg;
            break;
        case 'L':
            logFile = optarg;
            break;
        case '?':
            std::cerr << "Incorrect usage. Options are -h hostname -u username -p portnumber -c configfile -t testfile -L logfile\n";
            return 1;
        default:
            abort ();
        }

    //for now don't do anything, but presumably we could call stuff later. 
    std::cout << "Hostname: " << hostname << " Username: "<< username << " ServerPort: " << serverport << " configfile: "  
        << configFile << " TestFile: "<< testFile << " LogFile: " << logFile << "\n";

    //Make the socket.
    cs457::tcpClientSocket client(2000, std::string("127.0.0.1"));
    std::string message;
    while(true)
    {
        std::cout << "Input you message: ";
        std::cin >> message;
        client.sendString(message);
    }
    return 0;
}
