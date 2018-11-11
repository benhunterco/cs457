#include "client.h"

size_t cs457::client::send(std::string msg)
{
    //this should help to clean up the commands a littl bit. Automatically appends stuff.
    //automatically prepend username, we send on every message
    return sock->sendString(":" + username + " " + msg + "\r\n", true);
}

size_t cs457::client::registerUser()
{
    /**send appropriate registration details
     * first command is pass, which is @ if we haven't set one before.
     */
    std::string registration = "PASS " + password + "\r\n";
    return send(registration);
}


//this handles all of the user input commands.
//does only a couple things and checks a few for correct inpu.
int cs457::client::command(std::string command)
{
    int retVal = 1;
    // in this case, they've typed a command so it should be interpreted
    if (command[0] == '/')
    {
        //get rid of leading slash
        command = command.substr(1, command.length() - 1);
        Parsing::IRC_message msg(command + "\r\n");

        //quit breaks out of the command loop, as well as notifying the server politely 
        //   that we are quiting.
        if (msg.command == "QUIT")
        {
            retVal = 0;                                    //un continues.
            send(command); //sends string without slash.
        }
        //pretty straightforward, just prints help.
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
        //we chang our local variable of username as well as asking the server to do so,
        //in rcvCommand() we'll see what happens if the nick command is invalid.
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
        //boundary checking for kick
        else if (msg.command == "KICK")
        {
            if(msg.params.size() < 2){
                std::cout << "[CLIENT] <channelName> <userName> is required for /KICK. \n";
            }
            else
                send(command);
        }
        //boundary checking for oper.
        else if (msg.command == "OPER")
        {
            if(msg.params.size() < 2){
                std::cout << "[CLIENT] <userName> <password> is required for /OPER. \n";
            }
            else
                send(command);
        }
        //outputs the client version info, then asks the server for its version info
        else if (msg.command == "VERSION")
        {
            std::cout<< "Client version 1.0\n";
            send(command);
        }

        //if the command is a privmsg, save the recipient so next command will go to them
        //still send the privmsg to the server. Doesn't matter if its a channel or privmsg, save both.
        else if (msg.command == "PRIVMSG")
        {
            sendDirect = true;
            directUserOrChannel = msg.params[0];
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
        //this allows the user to just type and automatically wraps the message in an IRC message.
        if(sendDirect)
        {
            std::string sendString = " PRIVMSG " + directUserOrChannel + " :" + command + "\r\n";
            send(sendString);
        }
    }
    return retVal;
}


//this method recieves commands from the server.
//returns int so it can handle more than just true/false.
int cs457::client::rcvCommand()
{
    //recieving out of the socket. 
    std::string rcvMessage;
    int length;
    tie(rcvMessage, length) = sock->recvString();

    //if the socket is closed, return 0.
    //this handles the socket closing on us.
    if (length <= 0)
    {
        std::cout << "\n[CLIENT] Connection to remote host lost. Press Enter to continue." << std::endl;
        return 0;
    }
    //otherwise there was  something in the socket
    else
    {
        //parse it.
        Parsing::IRC_message message(rcvMessage);

        //Respond to the ping command by sending a pong.
        if (message.command == "PING")
            sock->sendString(":" + username + " PONG", true);

        //This is weird, but the server echos back the quit command to force this thread to quit. 
        //its an odd way of letting the treads talk to each other, but works.
        else if (message.command == "QUIT")
        {
            //Do not continue
            std::cout << "[CLIENT] Quiting..." << std::endl;
            return 0;
        }

        //So as we said above, there is interpretation of a nick recieval.
        //If the nick we asked for is taken or the password is wrong, the server rejects it.
        //then it sends this message back to change our nickname back. Weird but workds
        else if (message.command == "NICK")
        {
            username = message.params[0];
            std::cout << "\n[CLIENT] " << message.params[1] << "\n[CLIENT] Input Message or Command: " << std::flush;
        }

        //Message recieved from either a channel or user!
        else if (message.command == "PRIVMSG")
        {
            //Check to see if it was sent to a channel, if not its private.
            if (message.params[0][0] != '#')
            {

                //outputs from info
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
                //don't do anything with it if its to yourself!?!
            }
        }
        //This helped to clean up the version command, which was messy because it all arrived hectically
        else if (message.command == "VERSION")
        {
            std::cout <<"\n"<<message.params[0] << "\n[CLIENT] Input Message or Command: " << std::flush;
        }

        //basically same as privmsg
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

        //similar to privmsg, but with some extra info.
        else if (message.command == "KNOCK")
        {
            if(message.name == username)
            {
                //knock came back. either we don't need to knock or can't knock
            }
            else
            {
                std::string retStr = "\n[CLIENT] User " + message.name + " would like to join " + message.params[0] + ".";
                if(message.params.size() > 1)
                    retStr += " \nThey gave the message:\n\t" + message.params[1];
                retStr += "\n[CLIENT] If you would like to invite them, use /INVITE " + message.name + " " + message.params[0];
                std::cout << retStr << "\n[CLIENT] Inpue Message or Command: " << std::flush;
            }
        }

        //Interprets an invite from a user.
        else if (message.command == "INVITE")
        {
            std::cout << "\n[CLIENT] "<< message.name << " has invited you to join channel " << message.params[1] << "."
            << "\n[CLIENT] Input Message or Command: " << std::flush;
        }

        //handles the response of a topic query.
        //we don't get anything back when setting the topic
        else if (message.command == "TOPIC")
        {
            std::cout << "\n[CLIENT] topic for channel " << message.params[0] << " is " << message.params[1]
            << "\n[CLIENT] Input Message or Command: " << std::flush;
        }

        //This is another weird one. Only recieved on join failure.
        //will output the reason for join failure like password, invite only or ban.
        else if (message.command == "JOIN")
        {
            std::cout << "\n[CLIENT] " << message.params[0] << "\n[CLIENT] Input Message or Command: " << std::flush;
        }

        //lets you know if you were kicked from a channel.
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

        //similar to notice
        else if (message.command == "WALLOPS")
        {
            if (message.name != username)
            {
                std::cout << "\n[CLIENT] WALLOP'ing from " << message.name << ": " << message.params[0]
                          << "\n[CLIENT] Input Message or Command: " << std::flush;
            }
        }

        //lets you know who killed you, and ends the rcvcommand thread by returning 0.
        else if (message.command == "KILL")
        {
            std::cout << "\n[CLIENT] Your connection was killed by: " << message.name << " Press enter to continue." << std::endl;
            return 0;
        }

        //some commands are simple and don't require complex formating. 
        //in these cases, we just output them to the command line. 
        else
        {
            // just echo out what we recieved.
            std::cout << "\n[CLIENT] Recieved: " << rcvMessage << "[CLIENT] Input Message or Command: " << std::flush;
        }
        return 1;
    }
}