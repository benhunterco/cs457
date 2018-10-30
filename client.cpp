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

//Values passed into client from command line call.
//Not read from config file, that will be implemented later.
std::string hostname = "127.0.0.1";
std::string username = "bobby";
int serverport = 2000;
std::string configFile = "";
std::string testFile = "";
std::string logFile = "";

void clientRegister(cs457::tcpClientSocket *client)
{
    /**send appropriate registration details
     */
    std::string registration = "NICK " + username;
    client->sendString(registration, true);
}

//Here. Append nickname to front of command.
//so typing /time -> :bobby TIME
//          clientside conversion
//
void clientSend(cs457::tcpClientSocket *client)
{
    std::string input = "";
    while (input.find("QUIT") == std::string::npos) //See documentation for correct quiting command, Checks if quit is in the message. Add len=4?
    {
        //Make thread for sending and one for recieving.
        std::cout << "\ninput your message: ";
        //std::string input;
        getline(std::cin, input);
        //input += "\n"; << don't send if blank!
        if (input.length() > 0)
        {
            //in the future, we will see whether this is a command or message??
            //sends all messages in "<user>: what they typed" format.
            //if they have sent a command, then we just pass it along.
            //strip out the slash and preppend username:
            if (input[0] == '/')
            {
                //get rid of the slash.
                //send as an irc message
                input.erase(0,1);
                client->sendString(":" + username + " " +input + "\r\n", true);
            }else{
                //this is just a message. So send to the currently active channel.
                //client->sendString(us)
            }
        }
    }
}

void clientReceive(cs457::tcpClientSocket *client)
{
    std::string rcvMessage;
    while (rcvMessage.find("goodbye") == std::string::npos)
    {
        int length;
        tie(rcvMessage, length) = client->recvString();

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
                client->sendString("PONG", true);
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
    std::cout << "Hostname (default 127.0.0.1): " << hostname << " Username (default bobby): " << username << " ServerPort (default 2000): " << serverport << " configfile: "
              << configFile << " TestFile: " << testFile << " LogFile: " << logFile << "\n";

    //create the socket
    cs457::tcpClientSocket client(serverport, hostname);
    //register the user
    clientRegister(&client);
    std::thread sendThread(clientSend, &client);
    //clientSend(client);
    std::thread receiveThread(clientReceive, &client);
    sendThread.join();
    receiveThread.join();

    return 0;
}
