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
        command = command.substr(1, command.length() - 1);
        Parsing::IRC_message msg(command + "\r\n");
        if (msg.command == "QUIT")
        {
            retVal = 0;                                    //un continues.
            send(command); //sends string without slash.
        }
        else if (msg.command == "HELP")
        {
            std::cout << "************************************************************************************\n"
                      << "To see list of online users, type '/USERS'\n"
                      << "To send a message to a user, type '/PRIVMSG <user> :<your message here>'\n"
                      << "To create or join a channel, type '/JOIN <#channelName>'\n"
                      << "To see the created channels, type '/LIST'\n"
                      << "To send a message to a channel, type '/PRIVMSG <#channelName> :<your message here>'\n"
                      << "A more thorough explanation can be found in the readme file.\n"
                      << "************************************************************************************\n"
                      << std::flush;
        }
        else
        {
            //let the server deal with it.
            send(command);
        }
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
        else if (message.command == "PRIVMSG")
        {
            //Check to see if it was sent to a channel, if not its private.
            if (message.params[0][0] != '#')
            {

                std::cout << "\n[CLIENT] Message from " << message.name << ": " << message.params[1]
                          << "\n[CLIENT] Input Message or Command: " << std::flush;
            }
            else
            {
                //this came from a channel.
                if (message.name != username)
                {
                    std::cout << "\n[CLIENT] Message from " << message.name << " to channel " << message.params[0] << ": "
                              << message.params[1] << "\n[CLIENT] Input Message or Command: " << std::flush;
                }
                //don't do anything with it if its to yourself
            }
        }
        else
        {
            // just echo out what we recieved.
            std::cout << "\n[CLIENT] Recieved: " << rcvMessage << "[CLIENT] Input Message or Command: " << std::flush;
        }
        return 1;
    }
}