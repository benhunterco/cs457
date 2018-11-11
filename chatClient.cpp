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


//This method handles the input and sending of messages and runs on its own thread.
void clientSend()
{
    std::string input = "";
    int cont = 1;
    //while continue is true and recieve has not locked us out. 
    //This allows specific recieves to break out of sending.
    while (cont && sendAndRecieveLockOut) 
    {
        //get input from the user. 
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

//method for recieving of the client. 
//this is called in its own thread in main. 
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

    //parses the input arguments as described in the readme.
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

    //parse the config file.
    //Config file will overwrite the input parameters only if given. No default
    if(client.configFile.length() > 0)
    {
        //parsing stream for the file.
        std::ifstream config(client.configFile);
        if(config.is_open())
        {
            std::string line;
            while(getline(config, line))
            {
                //if it doesn't start with #, its not a comment. 
                if(line[0] != '#')
                {
                    //iss of the read in line. Allows us to split on spaces. 
                    std::istringstream iss(line);
                    std::string value; std::string attribute;
                    //we treat the first string as the attribute, the second as the value. 
                    iss >> attribute;
                    iss >> value;

                    //then, we look to see which attribute we found and set its value to what we read from the file. 
                    //disregards unrecognized attributes.
                    if(attribute == "last_server_used")
                        client.hostname = value;
                    else if(attribute == "port")
                        client.serverport = stoi(value);
                    //strings false and true must be used, cases matter.                   
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

    //print out read in stuff. 
    if(debug)
    std::cout << "Hostname (default 127.0.0.1): " << client.hostname << " Username (default bobby): " << client.username << " ServerPort (default 2000): " 
        << client.serverport << " configfile: "<< client.configFile << " TestFile: " << client.testFile << " LogFile: " << client.logFile << "\n";
    
    //this is the outer loop that acts as a starting point for the user. If they are disconnected, they fall back here. 
    bool cont = true;
    while (cont)
    {
        //basic intro help message
        std::cout << "[CLIENT] Use /CONNECT to connect to defaults, or /CONNECT <hostname> <port>, or /EXIT to exit \n"
                  << "[CLIENT] Use /CPASS to change the clients password for reconnect\n"
                  << "[CLIENT] Input Command: ";

        //read a line from the prompt
        std::string input;
        getline(std::cin, input);
        if (input.size() > 0)
        {
            //parse the input as a command and ignore the /.
            //user can't give messages a this point since it is not connected. 
            //treat the command as a irc_message.
            input = input.substr(1, input.length() - 1);
            input += "\r\n";
            Parsing::IRC_message msg(input);

            //Connect will connect to the server. 
            if (msg.command == "CONNECT")
            {
                //If we also supplied a hostname and port, connect to that instead of the input parameters.
                if (msg.params.size() == 2)
                {
                    client.serverport = stoi(msg.params[1]);
                    client.hostname = msg.params[0];
                }
                //open up a socket with supplied info.
                cs457::tcpClientSocket clientSock(client.serverport, client.hostname);
                //client object gets a pointer to the socket, so everthing can share it.
                //threads don't complain about pointers either, which used to be a problem for me.
                client.sock = &clientSock;
                //register the user. This call is not threaded.
                //Wait to verify that user is successfully registered.
                size_t success = client.registerUser();
                //this lets both the send an recieve threads go unhindered.
                sendAndRecieveLockOut = true;
                //start both threads and then wait on them.
                std::thread sendThread(clientSend);
                std::thread receiveThread(clientReceive);
                sendThread.join();
                receiveThread.join();
                //program is not done here however, we go through the loop again.
                //cont is still true!
            }

            //if the user would like to connect to something with a password, they will need the cpass command.
            //this is just an odd implementation detail.
            else if (msg.command == "CPASS")
            {
                client.password = msg.params[0];
            }
            //allows the user to fully exit. 
            else if (msg.command == "EXIT")
                cont = false;
        }
    }
    return 0;
}
