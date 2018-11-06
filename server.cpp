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

bool cs457::server::command(std::string msg, cs457::user &connectedUser)
{
    Parsing::IRC_message message(msg);

    //Handles the quit command. Disconnects the user and marks them as such.
    if (message.command == "QUIT")
    {
        connectedUser.userSocket.get()->sendString("QUIT");
        connectedUser.userSocket.get()->closeSocket();
        connectedUser.socketActive = false;
        std::cout << "[SERVER] Client " << connectedUser.getName() << " has disconnected" << endl;
        return false;
    }

    //Handles sending a private message
    //Will need to handle rooms in the future.
    //Currently, sends to one single user.
    else if (message.command == "PRIVMSG")
    {
        //std::cout << "private message recieved" << endl;
        cs457::user &rcvUser = getUser(message.params[0]);
        //in future, will be for loop for each user in params[0]
        if (rcvUser.socketActive)
        {
            rcvUser.userSocket.get()->sendString(message.params[1] + "\r\n");
            return true;
        }
        else
        {
            //IDK save message for later maybe? Send away message back?
            //std::cout << "USER NOT FOUND" << endl;
            connectedUser.userSocket.get()->sendString(rcvUser.getAwayMessage() + "\r\n");
            return true;
        }
    }

    //Handles the away command
    else if (message.command == "AWAY")
    {
        //std::cout<<"SETAWAY TO " + message.params[0] << endl;
        connectedUser.setAwayMessage(message.params[0]);
        return true;
    }

    //adds user to specified channel.
    //Channelnames start with # I believe
    else if (message.command == "JOIN")
    {
        std::string channelName = message.params[0];
        //add the pound if not already there.
        if(channelName[0] != '#'){
            channelName = "#" + channelName;
        }
        if (!addChannel(connectedUser, channelName))
        {
            //Add user to channel. Otherwise, user is the op.
            addUserToChannel(connectedUser, channelName);
            return true;
        }
        else
        {
            //do nothing, or perhaps return that the user created a channel successfuly
            return true;
        }
    }

    //returns list of channels to the user
    else if (message.command == "LIST"){
        std::string response = "Here is a list of active channels:\n";
        response += listChannels();
        connectedUser.userSocket.get()->sendString(response);
        return true;
    }

    else{
        std::cout << "unrecognized command "<< message.command << endl << "[SERVER]>";
        return true;
    }
    
}

cs457::user &cs457::server::addUserWithSocket(shared_ptr<cs457::tcpUserSocket> clientSocket)
{
    cs457::user connectedUser(clientSocket);
    addUser(connectedUser);
    //trying to more explicitily get the reference to the maps copy of the user.
    cs457::user &myref = userMap.at(connectedUser.getName());
    return myref;
}

bool cs457::server::addUserToChannel(cs457::user &requestingUser, std::string channelName, std::string pass /*= "@"*/)
{
    //first find the specified channel
    for (cs457::channel& c : channels)
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