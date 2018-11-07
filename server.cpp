#include "server.h"

bool cs457::server::addUser(cs457::user newUser)
{

    if (!userMap.count(newUser.getName()))
    {
        userMap.insert(std::make_pair(newUser.getName(), newUser));
        return true;
    }
    else
    {
        return false;
    }

    return false;
}

bool cs457::server::removeUser(cs457::user toRemove)
{
    if(userExists(toRemove.getName()))
    {
        userMap.erase(toRemove.getName());
        return true;
    }
    else
        return false;
}

bool cs457::server::addChannel(cs457::user requestingUser, std::string channelName)
{
    for (cs457::channel c : channels)
    {
        if (c.name == channelName)
        {
            return false;
        }
    }

    cs457::channel newChannel;
    newChannel.name = channelName;
    newChannel.members.push_back(requestingUser); //can use first member as op???
    newChannel.password = "@";
    channels.push_back(newChannel);
    return true;
}

std::map<std::string, cs457::user> cs457::server::getUsers()
{
    return userMap;
}

cs457::user &cs457::server::getUser(std::string user)
{
    if (userMap.find(user) != userMap.end())
    {
        return userMap.at(user);
    }
    else
    {
        throw; //some sort of exception, user not found.
    }
}

cs457::channel &cs457::server::getChannel(std::string &channelName)
{
    for (cs457::channel &c : channels)
    {
        if (c.name == channelName)
            return c;
    }
    throw "Channel " + channelName + " not found";
}

int cs457::server::command(std::string msg, cs457::user &connectedUser)
{
    Parsing::IRC_message message(msg);

    //go commit die
    if (message.command == "DIE")
    {
        if (connectedUser.getLevel() == "sysop")
        {
            //write out info to files here.
            //graceful template exists. leave this for now.
            abort();//probably the gnarliest way of doing it.
        }
        return 0;
    }

    //Handles the quit command. Disconnects the user and marks them as such.
    else if (message.command == "QUIT")
    {
        connectedUser.userSocket.get()->sendString("QUIT");
        connectedUser.userSocket.get()->closeSocket();
        connectedUser.socketActive = false;
        std::cout << "[SERVER] Client " << connectedUser.getName() << " has disconnected" << endl;
        return 1;
    }

    //Handles recieving and sending messages.
    //For both users and channels.
    //Easily the biggest command that the server needs to handle.
    else if (message.command == "PRIVMSG")
    {
        //std::cout << "private message recieved" << endl;
        std::string desinationStrings = message.params[0];
        //Why on earth is there not a standard way of doing this c++??
        //Split on the commas.
        std::string token;
        std::vector<std::string> destinations;
        std::istringstream tStream(desinationStrings);
        while (std::getline(tStream, token, ','))
        {
            destinations.push_back(token);
        }

        //for each destination in the vector, either send to the channel or user.
        for (std::string recipient : destinations)
        {
            if (recipient[0] == '#')
            {
                //do the channel sending.
                try
                {
                    //get the channel by name.
                    cs457::channel rcvChannel = getChannel(recipient);
                    //send to each member of the channel.
                    for (cs457::user &rcvUser : rcvChannel.members)
                    {
                        if (rcvUser.socketActive && !rcvUser.i)
                        {
                            //does pretty much the same as below. Client should check recipient to see if its a channel then?
                            std::string sendString = ":" + message.name + " PRIVMSG " + recipient + " :" + message.params[1] + "\r\n";
                            //sends the recipient the string, although we take out other recipients.
                            //Might not be necessary. Maybe only need to strip channels?
                            rcvUser.userSocket.get()->sendString(sendString);
                        }
                    }
                }
                catch (std::string e)
                {
                    //Channel not found. Can just ignore for now.
                }
            }
            else
            {
                //CHECK TO SEE IF USER EXISTS!!!
                cs457::user &rcvUser = getUser(recipient);
                //in future, will be for loop for each user in params[0]
                //check to see if they're online and visible
                if (rcvUser.socketActive && !rcvUser.i)
                {
                    std::string sendString = ":" + message.name + " PRIVMSG " + recipient + " :" + message.params[1] + "\r\n";
                    //sends the recipient the string, although we take out other recipients.
                    //Might not be necessary. Maybe only need to strip channels?
                    rcvUser.userSocket.get()->sendString(sendString);
                }
                else 
                {
                    //IDK save message for later maybe? Send away message back?
                    //std::cout << "USER NOT FOUND" << endl;
                    connectedUser.userSocket.get()->sendString(rcvUser.getAwayMessage() + "\r\n");
                }
            }
        }
        return 2;
    }

    //Same as privmsg without any returns.
    else if (message.command == "NOTICE")
    {
        //std::cout << "private message recieved" << endl;
        std::string desinationStrings = message.params[0];
        //Why on earth is there not a standard way of doing this c++??
        //Split on the commas.
        std::string token;
        std::vector<std::string> destinations;
        std::istringstream tStream(desinationStrings);
        while (std::getline(tStream, token, ','))
        {
            destinations.push_back(token);
        }

        //for each destination in the vector, either send to the channel or user.
        for (std::string recipient : destinations)
        {
            if (recipient[0] == '#')
            {
                //do the channel sending.
                try
                {
                    //get the channel by name.
                    cs457::channel rcvChannel = getChannel(recipient);
                    //send to each member of the channel.
                    for (cs457::user &rcvUser : rcvChannel.members)
                    {
                        if (rcvUser.socketActive && rcvUser.s)
                        {
                            //does pretty much the same as below. Client should check recipient to see if its a channel then?
                            std::string sendString = ":" + message.name + " NOTICE " + recipient + " :" + message.params[1] + "\r\n";
                            //sends the recipient the string, although we take out other recipients.
                            //Might not be necessary. Maybe only need to strip channels?
                            rcvUser.userSocket.get()->sendString(sendString);
                        }
                    }
                }
                catch (std::string e)
                {
                    //Channel not found. Can just ignore for now.
                }
            }
            else
            {
                //CHECK TO SEE IF USER EXISTS!!!
                cs457::user &rcvUser = getUser(recipient);
                //in future, will be for loop for each user in params[0]
                if (rcvUser.socketActive && rcvUser.s)
                {
                    std::string sendString = ":" + message.name + " NOTICE " + recipient + " :" + message.params[1] + "\r\n";
                    //sends the recipient the string, although we take out other recipients.
                    //Might not be necessary. Maybe only need to strip channels?
                    rcvUser.userSocket.get()->sendString(sendString);
                }
                else
                {
                    //do nothing cause its notice.
                }
            }
        }
        return 2;
    }

    //Handles the away command
    else if (message.command == "AWAY")
    {
        //std::cout<<"SETAWAY TO " + message.params[0] << endl;
        connectedUser.setAwayMessage(message.params[0]);
        return 2;
    }

    //adds user to specified channel.
    //Channelnames start with # I believe
    else if (message.command == "JOIN")
    {
        std::string channelName = message.params[0];
        //add the pound if not already there.
        if (channelName[0] != '#')
        {
            channelName = "#" + channelName;
        }
        if (!addChannel(connectedUser, channelName))
        {
            //Add user to channel. Otherwise, user is the op.
            addUserToChannel(connectedUser, channelName);
            return 2;
        }
        else
        {
            //do nothing, or perhaps return that the user created a channel successfuly
            return 2;
        }
    }

    //returns list of channels to the user
    else if (message.command == "LIST")
    {
        std::string response = "Here is a list of active channels:\n";
        response += listChannels() + "\r\n";
        connectedUser.userSocket.get()->sendString(response);
        return 2;
    }

    //Returns info of the server. How the heck would you get compilation of server?
    else if (message.command == "INFO")
    {
        double seconds = difftime(time(NULL), startTime);
        int rounded = (int)seconds;
        std::string response = "Server info: \n\t*Uptime: " + std::to_string(rounded) +
                               " seconds.\n\t*Compilation: recently\r\n";
        connectedUser.userSocket.get()->sendString(response);
        return 2;
    }

    //Sends an invite to the requested user. Really its just a message.
    else if (message.command == "INVITE")
    {
        std::string recipient = message.params[0];
        cs457::user &rcvUser = getUser(recipient);

        if (rcvUser.socketActive)
        {
            std::string sendString = ":" + message.name + " INVITE " + recipient + " :" + message.params[1] + "\r\n";
            //sends the recipient the string, although we take out other recipients.
            //Might not be necessary. Maybe only need to strip channels?
            rcvUser.userSocket.get()->sendString(sendString);
        }
        else
        {
            connectedUser.userSocket.get()->sendString(rcvUser.getAwayMessage() + "\r\n");
        }
        return 2;
    }

    //checks to see if user is online.
    else if (message.command == "ISON")
    {
        if (userOnline(message.params[0]))
        {
            connectedUser.userSocket.get()->sendString("User: " + message.params[0] + ", is online.\r\n");
        }
        else if (userExists(message.params[0])) //check if we've seen him.
        {
            connectedUser.userSocket.get()->sendString("User: " + message.params[0] + ", is offline.\r\n");
        }
        else
        {
            connectedUser.userSocket.get()->sendString("User: " + message.params[0] + ", has never been online.\r\n");
        }
        return true;
    }

    //sets the user to a sysop
    else if (message.command == "OPER")
    {
        //password = notagoodpassword
        if (message.params[1] == "notagoodpassword")
        {
            if (userExists(message.params[0]))
            {
                cs457::user &promoted = getUser(message.params[0]);
                promoted.setLevel("sysop");
            }
        }
        else
        {
            connectedUser.userSocket.get()->sendString("Wrong password or username! \r\n");
        }
        return 2;
    }

    //lets the user change their name if it isn't someone elses
    else if (message.command == "NICK")
    {
        //check to see if someones used this name.
        if(!userExists(message.params[0]))
        {
            //get old user out of map.
            std::string oldName = connectedUser.getName();
            cs457::user newUser = getUser(oldName);
            removeUser(oldName);
            newUser.setName(message.params[0]);
            addUser(newUser);
        }
        else 
        {
            //send back a nick command, which will change the clients name back.
            connectedUser.userSocket.get()->sendString("NICK " + connectedUser.getName()
             +" :" + message.params[0] + " is taken!\r\n");
        }
        return 2;
    }

    //allows the user to change their mode
    else if (message.command == "MODE")
    {
        for (std::string mode : message.params)
        {
            //switch on the first char, which should be one of the modes
            switch(mode[0]){
                case 'i':
                    connectedUser.i = !connectedUser.i; //just toggles the boolean.
                    break;
                case 's':
                    connectedUser.s = !connectedUser.s; //just toggles the boolean.
                    break;
                case 'w':
                    connectedUser.w = !connectedUser.w; //just toggles the boolean.
                    break;
                default:
                    connectedUser.userSocket.get()->sendString("Unrecognized mode attribute: " + mode +".\r\n");
            }
        }
        return 2;
    }
    //Kicks the user if the requester is a channel operator or above.
    //Channelop is lowest. We don't really need to grant this, creator is channelop
    else if (message.command == "KICK")
    {
        if (userExists(message.params[1]))
        {
            //get the channel that requires kicking.
            try
            {
                cs457::channel &chan = getChannel(message.params[0]);

                //check to see if requester is channelop. This requires him being first in the list.
                //he could also be a sysop. Haven't really done admin yet.
                if (chan.members[0].getName() == message.name || connectedUser.getLevel() == "sysop")
                {
                    for (int i = 0; i < chan.members.size(); i++)
                    {
                        if (chan.members[i].getName() == message.params[1])
                        {
                            chan.members.erase(chan.members.begin() + i); //I guess I should know how iterators work.
                            cs457::user &victim = getUser(message.params[1]);
                            //forward the kicking command. Kick from any remaining local representation and let the user know.
                            victim.userSocket.get()->sendString(msg);
                            return 2;
                        }
                    }
                    //Could not find user in channel. Might not need to let user know.
                }
                else
                {
                    //insufficient permissions, let them know that one.
                    connectedUser.userSocket.get()->sendString("You do not have permission to kick in this channel!\r\n");
                    return 2;
                }
            }
            catch (std::string error)
            {
                connectedUser.userSocket.get()->sendString(error + "\r\n");
                return 2;
            }
        }
        return 2;
    }

    //disconnects the user by closing they're socket. Could be implemented with the client, but this might be simpler.
    //kill only takes one parameter. Some users are allowed to do it.
    else if (message.command == "KILL")
    {
        if (connectedUser.getLevel() == "sysop")
        {
            if (userOnline(message.params[0]))
            {
                cs457::user &victim = getUser(message.params[0]);
                victim.userSocket.get()->sendString("You have been killed!\r\n");
                //victim.closeSocket();
                std::cout << victim.closeSocket();
                return 2;
            }
        }
        else
        {
            connectedUser.userSocket.get()->sendString("You do not have license to kill!\r\n");
        }
        return 2;
    }
    else
    {
        std::cout << "unrecognized command " << message.command << endl
                  << "[SERVER]>";
        return 2;
    }
}

cs457::user &cs457::server::addUserWithSocket(shared_ptr<cs457::tcpUserSocket> clientSocket)
{
    cs457::user connectedUser(clientSocket);
    if (!userExists(connectedUser.getName()))
    {
        addUser(connectedUser);
        cs457::user &myref = userMap.at(connectedUser.getName());
        return myref;
    }
    else
    {
        cs457::user& returnedUser = getUser(connectedUser.getName());
        returnedUser.setSocket(clientSocket);
        returnedUser.socketActive = true;
        return returnedUser;
    }
    //trying to more explicitily get the reference to the maps copy of the user.
}

bool cs457::server::addUserToChannel(cs457::user &requestingUser, std::string channelName, std::string pass /*= "@"*/)
{
    //first find the specified channel
    for (cs457::channel &c : channels)
    {
        if (c.name == channelName)
        {
            //check password here!!!
            //then add user to channel.
            c.members.push_back(requestingUser);
            return true;
        }
    }

    //channel not found???
    return false;
}

std::string cs457::server::listChannels(bool showUsers /*= false*/)
{
    std::string list;
    for (cs457::channel c : channels)
    {
        list += c.name + "\n";
        if (showUsers)
        {
            for (cs457::user u : c.members)
            {
                list += ("\t" + u.getName() + "\n");
            }
        }
    }
    return list;
}

bool cs457::server::userExists(std::string userName)
{
    if (userMap.count(userName))
        return true;
    else
        return false;
}

bool cs457::server::userOnline(std::string userName)
{
    if (userExists(userName))
    {
        cs457::user &user = getUser(userName);
        return user.socketActive;
    }
    else
        return false;
}