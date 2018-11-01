#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <thread>
#include "tcpUserSocket.h"
#include "tcpClientSocket.h"
#include "Parsing.h"
#include "client.h"

//Values passed into client from command line call.
cs457::client client;
void clientRegister(cs457::tcpClientSocket *clientSock)
{
    /**send appropriate registration details
     */
    std::string registration = "NICK " + client.username + "\r\n";
    clientSock->sendString(registration, true);
}

//Here. Append nickname to front of command.
//so typing /time -> :bobby TIME
//          clientside conversion
//
void clientSend(cs457::tcpClientSocket *clientSock)
{
    std::string input = "";
    bool cont = true;
    while (cont) //See documentation for correct quiting command, Checks if quit is in the message. Add len=4?
    {
        //Make thread for sending and one for recieving.
        std::cout << "\ninput your message: ";
        getline(std::cin, input);
        
        if (input.length() > 0)
        {
            //strip out the slash and preppend username:
            if (input[0] == '/')
            {
                //do local parsing of command, if anything needs to be updated we will.
                //get rid of the slash.
                //send as an irc message
                input.erase(0,1);
                clientSock->sendString(":" + client.username + " " +input + "\r\n", true);
            }else{
                //this is just a message. So send to the currently active channel.
                //may just keep track of one channel somehow...
                //client->sendString(us)
            }
        }
    }
}

void clientReceive(cs457::tcpClientSocket *clientSock)
{
    std::string rcvMessage;
    while (rcvMessage.find("goodbye") == std::string::npos)
    {
        int length;
        tie(rcvMessage, length) = clientSock->recvString();

        if (length <= 0)
        {
            std::cout << "socket close!" << std::endl;
            break;
        }
        //Handle commands, anything that starts with /
        if (length > 0)
        {
            Parsing::IRC_message message(rcvMessage);

            //Respond to the ping command by sending a pong.
            if (message.command == "PING")
                clientSock->sendString("PONG", true);
        }
        std::cout << "\n"
                  << rcvMessage << std::endl;
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
            client.hostname = optarg;
            break;
        case 'u':
            client.username = optarg;
            break;
        case 'p':
            client.serverport = atoi(optarg);
            if (client.serverport == 0)
            {
                std::cerr << "Incorrect port number. Please enter an integer\n";
                return 1;
            }
            break;
        case 'c':
            client.configFile = optarg;
            break;
        case 't':
            //if this is set, we run the test
            client.testFile = optarg;
            break;
        case 'L':
            client.logFile = optarg;
            break;
        case '?':
            std::cerr << "Incorrect usage. Options are -h hostname -u username -p portnumber -c configfile -t testfile -L logfile\n";
            return 1;
        default:
            abort();
        }

    //for now don't do anything, but presumably we could call stuff later.
    std::cout << "Hostname (default 127.0.0.1): " << client.hostname << " Username (default bobby): " << client.username << " ServerPort (default 2000): " << client.serverport << " configfile: "
              << client.configFile << " TestFile: " << client.testFile << " LogFile: " << client.logFile << "\n";

    //create the socket
    cs457::tcpClientSocket clientSock(client.serverport, client.hostname);
    //register the user
    clientRegister(&clientSock);
    std::thread sendThread(clientSend, &clientSock);
    //clientSend(client);
    std::thread receiveThread(clientReceive, &clientSock);
    sendThread.join();
    receiveThread.join();

    return 0;
}
