#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include "tcpUserSocket.h"
#include "tcpClientSocket.h"

//Values passed into client from command line call.
//Not read from config file, that will be implemented later.
std::string hostname = "";
std::string username = "";
int serverport = 0;
std::string configFile = "";
std::string testFile = "";
std::string logFile = "";

int main(int argc, char **argv)
{

    opterr = 0;
    char c = ' ';
    while ((c = getopt(argc, argv, "h:u:p:c:t:L:")) != -1)
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
            if (serverport == 0)
            {
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
            abort();
        }

    //for now don't do anything, but presumably we could call stuff later.
    std::cout << "Hostname: " << hostname << " Username: " << username << " ServerPort: " << serverport << " configfile: "
              << configFile << " TestFile: " << testFile << " LogFile: " << logFile << "\n";
    
    //create the socket
    cs457::tcpClientSocket client(serverport, hostname);
    while(true)
    {
        std::cout << "input your message: ";
        std::string input;
        getline(std::cin, input);
        input += "\n";
        client.sendString(input, true);
    }
        
    return 0;
}
