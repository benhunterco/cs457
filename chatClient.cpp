#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <thread>
#include <fstream>
//#include <istringstream>
#include "tcpUserSocket.h"
#include "tcpClientSocket.h"
#include "Parsing.h"
#include "client.h"

//Here is the client object. It does all the keeping track of statey stuff.
cs457::client client;

//bool that lets send an recieve communicate and close out each other.
bool sendAndRecieveLockOut = true;

//bool that silences or allows debug commands.
bool debug = false;
//Here. Append nickname to front of command.
//so typing /time -> :bobby TIME
//          clientside conversion
//
void clientSend()
{
    std::string input = "";
    int cont = 1;
    while (cont && sendAndRecieveLockOut) //See documentation for correct quiting command, Checks if quit is in the message. Add len=4?
    {
        //Make thread for sending and one for recieving.
        std::cout << "[CLIENT] Input Message or Command: ";
        getline(std::cin, input);

        if (input.length() > 0 && sendAndRecieveLockOut)
        {
            cont = client.command(input); //return is an int. For now it could be bool though
        }
    }
    // std::cout << "OUT of send\n";
    sendAndRecieveLockOut = false;
}

void clientReceive()
{
    int cont = 1;
    while (cont && sendAndRecieveLockOut)
    {
        cont = client.rcvCommand();
    }
    //std::cout << "OUT of rcv\n";
    sendAndRecieveLockOut = false;

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

    if(client.configFile.length() > 0)
    {
        //we got to parse that bad boy.
        std::ifstream config(client.configFile);
        if(config.is_open())
        {
            std::string line;
            while(getline(config, line))
            {
                if(line[0] != '#')
                {
                    std::istringstream iss(line);
                    std::string value; std::string attribute;
                    iss >> attribute;
                    iss >> value;
                    if(attribute == "last_server_used")
                        client.hostname = value;
                    else if(attribute == "port")
                        client.serverport = stoi(value);
                    else if (attribute == "default_debug_mode")
                    {
                        if(value == "false")
                        {
                            debug = false;
                            client.debug = false;
                        }
                        else{
                            debug = true;
                            client.debug = true;
                        }
                    }
                    else if (attribute == "log")
                    {
                        if(value == "false")
                        {
                            client.log = false;
                        }
                        else{
                            client.log = true;
                        }
                    }
                    else if (attribute == "default_log_file")
                    {
                        client.logFile = value;
                    }
                }
            }
        }
        else   
            std::cerr << "file could not be opened";
    }

    if(debug)
    std::cout << "Hostname (default 127.0.0.1): " << client.hostname << " Username (default bobby): " << client.username << " ServerPort (default 2000): " 
        << client.serverport << " configfile: "<< client.configFile << " TestFile: " << client.testFile << " LogFile: " << client.logFile << "\n";
    //Will allow the user to connect.
    bool cont = true;
    while (cont)
    {
        std::cout << "[CLIENT] Use /CONNECT to connect to defaults, or /CONNECT <hostname> <port>, or /EXIT to exit \n"
                  << "[CLIENT] Use /CPASS to change the clients password for reconnect\n"
                  << "[CLIENT] Input Message or Command: ";
        std::string input;
        getline(std::cin, input);
        if (input.size() > 0)
        {
            input = input.substr(1, input.length() - 1);
            input += "\r\n";
            Parsing::IRC_message msg(input);
            if (msg.command == "CONNECT")
            {
                if (msg.params.size() == 2)
                {
                    client.serverport = stoi(msg.params[1]);
                    client.hostname = msg.params[0];
                }
                cs457::tcpClientSocket clientSock(client.serverport, client.hostname);
                client.sock = &clientSock;
                //register the user. This call is not threaded.
                //Wait to verify that user is successfully registered.
                size_t success = client.registerUser();
                sendAndRecieveLockOut = true;
                std::thread sendThread(clientSend);
                //clientSend(client);
                std::thread receiveThread(clientReceive);
                sendThread.join();
                receiveThread.join();
            }
            else if (msg.command == "CPASS")
            {
                client.password = msg.params[0];
            }
            else if (msg.command == "EXIT")
                cont = false;
        }
    }
    return 0;
}
