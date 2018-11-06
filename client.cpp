#include "client.h"

size_t cs457::client::send(std::string msg)
{
    //this should help to clean up the commands a littl bit. Automatically appends stuff.
    return sock->sendString(":" + username + " " + msg + "\r\n", true);
}

size_t cs457::client::registerUser()
{
    /**send appropriate registration details
     */
    std::string registration = "NICK " + username + "\r\n";
    return send(registration);
    //possibly do passwordy stuff here.
}

int cs457::client::command(std::string command)
{
    int retVal = 1;
    if (command[0] == '/')
    {
        send(command.substr(1, command.length() - 1)); //sends string without slash.
        if (command.substr(1, 4) == "QUIT")            //un continues.
            retVal = 0;
    }
    else
    {
        //send to active channel.
    }
    return retVal;
}

int cs457::client::rcvCommand()
{
    std::string rcvMessage;
    int length;
    tie(rcvMessage, length) = sock->recvString();

    //if the socket is closed, rerurn 0.
    if (length <= 0)
    {
        std::cout << "\n[CLIENT] Connection to remote host lost. Use /CONNECT to attempt reconnect. \n";
        std::cout << "[CLIENT] Input Message or Command: " << std::flush;
        return 0;
    }
    else
    {
        Parsing::IRC_message message(rcvMessage);

        //Respond to the ping command by sending a pong.
        if (message.command == "PING")
            sock->sendString("PONG", true);

        else if (message.command == "QUIT")
        {
            //Do not continue
            std::cout << "[CLIENT] Quiting..." << std::endl;
            return 0;
        }
        else
        {
            // just echo out what we recieved.
            std::cout << "\n[CLIENT] Recieved: " << rcvMessage << "[CLIENT] Input Message or Command: "<< std::flush;
        }
        return 1;
    }
}