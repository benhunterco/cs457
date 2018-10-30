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
    return true;
}

std::map<std::string, cs457::user> cs457::server::getUsers()
{
    return userMap;
}

cs457::user& cs457::server::getUser(std::string user)
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

bool cs457::server::command(std::string msg, cs457::user& connectedUser)
{
    Parsing::IRC_message message(msg);
std::cout << "HERe" <<endl;
    //Handles the quit command. Disconnects the user and marks them as such.
    if (message.command == "QUIT")
    {
        connectedUser.userSocket.get()->sendString("goodbye");
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
        cs457::user& rcvUser = getUser(message.params[0]);
        //in future, will be for loop for each user in params[0]
        if (rcvUser.socketActive)
        {
            rcvUser.userSocket.get()->sendString(message.params[1] + "\r\n");
            return true;
        }else{
            //IDK save message for later maybe? Send away message back?
            std::cout << "USER NOT FOUND" << endl;
            connectedUser.userSocket.get()->sendString(rcvUser.getAwayMessage());
            return true;
        }
    }

    //Handles the away command
    else if (message.command == "AWAY")
    {
        std::cout<<"SETAWAY TO " + message.params[0] << endl;
        connectedUser.setAwayMessage(message.params[0]);
        return true;
    }

    return false;
}

cs457::user& cs457::server::addUserWithSocket(shared_ptr<cs457::tcpUserSocket> clientSocket){
    cs457::user connectedUser(clientSocket);
    addUser(connectedUser);
    //trying to more explicitily get the reference to the maps copy of the user.
    cs457::user& myref = userMap.at(connectedUser.getName());
    return myref;
}