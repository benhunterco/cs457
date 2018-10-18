#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <thread>
#include "tcpUserSocket.h"
#include "tcpClientSocket.h"

//Values passed into client from command line call.
//Not read from config file, that will be implemented later.
std::string hostname = "127.0.0.1";
std::string username = "";
int serverport = 2000;
std::string configFile = "";
std::string testFile = "";
std::string logFile = "";

void clientSend(cs457::tcpClientSocket *client)
{
    std::string input = "";
    while (input.find("EXIT") == std::string::npos) //See documentation for correct quiting command, Checks if quit is in the message. Add len=4?
    {
        //Make thread for sending and one for recieving.
        std::cout << "input your message: ";
        std::string input;
        getline(std::cin, input);
        //input += "\n";
        client->sendString(input, true);
    }
}

void clientReceive(cs457::tcpClientSocket *client)
{
    std::string rcvMessage;
    while (rcvMessage.find("goodbye") == std::string::npos)
    {
        int length;
        tie(rcvMessage, length) = client->recvString();
        std::cout << "\n" << rcvMessage;
    }
}

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
    std::cout << "Hostname (default 127.0.0.1): " << hostname << " Username: " << username << " ServerPort (default 2000): " << serverport << " configfile: "
              << configFile << " TestFile: " << testFile << " LogFile: " << logFile << "\n";

    //create the socket
    cs457::tcpClientSocket client(serverport, hostname);
    std::thread sendThread(clientSend, &client);
    //clientSend(client);
    std::thread receiveThread(clientReceive, &client);
    sendThread.join();
    receiveThread.join();

    return 0;
}
