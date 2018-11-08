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
    std::string registration = "PASS " + password + "\r\n";
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
        else if (msg.command == "NICK")
        {
            if(msg.params.size() < 1)
                std::cout << "[CLIENT] <userName> is required for /NICK. \n";
            else 
            {
                username = msg.params[0];
                send(command);
            }
        }
        else if (msg.command == "KICK")
        {
            if(msg.params.size() < 2){
                std::cout << "[CLIENT] <channelName> <userName> is required for /KICK. \n";
            }
            else
                send(command);
        }
        else if (msg.command == "OPER")
        {
            if(msg.params.size() < 2){
                std::cout << "[CLIENT] <userName> <password> is required for /KICK. \n";
            }
            else
                send(command);
        }
        else if (msg.command == "VERSION")
        {
            std::cout<< "Client version 1.0\n";
            send(command);
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
        std::cout << "\n[CLIENT] Connection to remote host lost. Press Enter to continue." << std::endl;
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

        else if (message.command == "NICK")
        {
            username = message.params[0];
            std::cout << "\n[CLIENT] " << message.params[1] << "\n[CLIENT] Input Message or Command: " << std::flush;
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
        else if (message.command == "VERSION")
        {
            std::cout <<"\n"<<message.params[0] << "\n[CLIENT] Input Message or Command: " << std::flush;
        }
        else if (message.command == "NOTICE")
        {
            //Check to see if it was sent to a channel, if not its private.
            if (message.params[0][0] != '#')
            {

                std::cout << "\n[CLIENT] Notice from " << message.name << ": " << message.params[1]
                          << "\n[CLIENT] Input Message or Command: " << std::flush;
            }
            else
            {
                //this came from a channel.
                if (message.name != username)
                {
                    std::cout << "\n[CLIENT] Notice from " << message.name << " to channel " << message.params[0] << ": "
                              << message.params[1] << "\n[CLIENT] Input Message or Command: " << std::flush;
                }
                //don't do anything with it if its to yourself
            }
        }
        else if (message.command == "INVITE")
        {
            std::cout << "\n[CLIENT] "<< message.name << " has invited you to join channel " << message.params[1] << "."<<std::endl;
        }
        else if (message.command == "TOPIC")
        {
            std::cout << "\n[CLIENT] topic for channel " << message.params[0] << " is " << message.params[1]
            << "\n[CLIENT] Input Message or Command: " << std::flush;
        }
        else if (message.command == "KICK")
        {
            std::cout << "\n[CLIENT] "<< message.name << " has kicked you from channel: " << message.params[0]<< ".";
            if(message.params.size() > 2){
                std::cout << " Reason: "+ message.params[2] << std::endl;
            }
            else{
                std::cout << " No reason given." << std::endl;
            }
            std::cout<< "\n[CLIENT] Input Message or Command: " << std::flush;
        }
        else if (message.command == "WALLOPS")
        {
            if (message.name != username)
            {
                std::cout << "\n[CLIENT] WALLOP'ing from " << message.name << ": " << message.params[0]
                          << "\n[CLIENT] Input Message or Command: " << std::flush;
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