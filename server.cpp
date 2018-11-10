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

bool cs457::server::userInChannel(cs457::user& check, cs457::channel& chan)
{
    for(cs457::user& member : chan.members)
    {
        if(member.getName() == check.getName())
            return true;
    }
    return false;
}

bool cs457::server::removeUser(cs457::user toRemove)
{
    if (userExists(toRemove.getName()))
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
    newChannel.userStatusMap[requestingUser.getName()].o = true;
    newChannel.password = "@";
    channels.push_back(newChannel);
    return true;
}

void cs457::server::writeUsers()
{
    remove((dbPath + "users.txt").c_str());
    std::ofstream myfile (dbPath + "users.txt");
    if(myfile.is_open())
    {
        for(auto u : userMap)
        {
            myfile << u.second.toString() + "\n";
        }
        myfile.close();
    }
}

void cs457::server::addUserFromFile(std::string fileLine)
{
    std::istringstream iss(fileLine);
    std::string uName; std::string pass; std::string level; std::string banned;
    iss >> uName;                                        
    iss >> pass;
    iss >> level;
    iss >> banned;
    if(banned == "true")
        bannedUsers.push_back(uName);
    //cout << banned;
    addUser(cs457::user(uName, pass, level, banned)); 
}
bool cs457::server::readUsers()
{
    std::string line;
    ifstream userStream(dbPath + "users.txt");
    if(userStream.is_open())
    {
        while(getline(userStream, line))
        {
            addUserFromFile(line);
        }
    }
    return false;
}

void cs457::server::writeBans()
{
    remove((dbPath + "banusers.txt").c_str());
    std::ofstream myfile (dbPath + "banusers.txt");
    if(myfile.is_open())
    {
        for(std::string u : bannedUsers)
        {
            myfile << u + "\n";
        }
        myfile.close();
    }
}

void cs457::server::writeChannels()
{
    remove((dbPath + "channels.txt").c_str());
    std::ofstream myfile (dbPath + "channels.txt");
    if(myfile.is_open())
    {
        for(cs457::channel c : channels)
        {
            myfile << c.name << " ";
            if(c.password.size() > 0)
                myfile << c.password;
            else
                myfile << "@";
            myfile << "\n";

        }
        myfile.close();
    }
}

bool cs457::server::readBanner()
{
    banner.erase();//empty the current banner.
    std::string line;
    ifstream bannerFile(dbPath + "banner.txt");
    if (bannerFile.is_open())
    {
        while (getline(bannerFile, line))
        {
            banner += line + "\n";
        }
        std::cout << banner;
        return true;
    }
    else 
        return false;
}

bool plusorminus(char pom)
{
    if (pom == '+')
        return true;
    else
        return false;
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
        throw std::string("User " + user + " not found."); //some sort of exception, user not found.
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
            abort(); //probably the gnarliest way of doing it.
        }
        return 0;
    }

    //Handles the quit command. Disconnects the user and marks them as such.
    else if (message.command == "QUIT")
    {
        connectedUser.userSocket.get()->sendString("QUIT");
        connectedUser.userSocket.get()->closeSocket();
        connectedUser.socketActive = false;
        std::cout << "\n[SERVER] Client " << connectedUser.getName() << " has disconnected";
        return 1;
    }

    //allows user to exit from space seperated list of channels.
    else if (message.command == "PART")
    {
        for (std::string channelName : message.params)
        {
            try
            {
                channel &chan = getChannel(channelName);
                for (int i = 0; i < chan.members.size(); i++)
                {
                    if (chan.members[i].getName() == connectedUser.getName())
                    {
                        chan.members.erase(chan.members.begin() + i); //I guess I should know how iterators work.
                        return 2;
                    }
                }
            }
            catch (std::string error)
            {
                //do something?
            }
        }
        return 2;
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
                    cs457::channel& rcvChannel = getChannel(recipient);
                    //send to each member of the channel.
                
                    for (cs457::user &rcvUser : rcvChannel.members)
                    {
                        if (rcvUser.socketActive && !rcvUser.i && (!rcvChannel.n || userInChannel(connectedUser, rcvChannel))
                        && !rcvUser.silencedUsers[connectedUser.getName()])
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
                if (rcvUser.socketActive && !rcvUser.i && !rcvUser.silencedUsers[connectedUser.getName()])
                {
                    std::string sendString = ":" + message.name + " PRIVMSG " + recipient + " :" + message.params[1] + "\r\n";
                    //sends the recipient the string, although we take out other recipients.
                    //Might not be necessary. Maybe only need to strip channels?
                    rcvUser.userSocket.get()->sendString(sendString);
                }
                else if (!rcvUser.silencedUsers[connectedUser.getName()])
                {
                    //sends away message if the user is away. But not if they silenced.
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
                        if (rcvUser.socketActive && rcvUser.s && (!rcvChannel.n || userInChannel(connectedUser, rcvChannel))
                        && !rcvUser.silencedUsers[connectedUser.getName()])
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
                if (rcvUser.socketActive && rcvUser.s && !rcvUser.silencedUsers[connectedUser.getName()])
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

    //same as notice except it sends only to those with w set or sysop level
    else if (message.command == "WALLOPS")
    {
        //Look through all users and send to w's and sysops.
        for (auto u : getUsers())
        {
            if ((u.second.w || u.second.getLevel() == "sysop") && u.second.socketActive)
            {
                u.second.userSocket.get()->sendString(":" + connectedUser.getName() + " WALLOPS :" + message.params[0] + "\r\n");
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
            bool succ;
            if (message.params.size() > 1)
                succ = addUserToChannel(connectedUser, channelName, message.params[1]);
            //Add user to channel. Otherwise, user is the op.
            else
                succ = addUserToChannel(connectedUser, channelName);
            if (!succ)
            {
                connectedUser.userSocket.get()->sendString(":" + connectedUser.getName() + " JOIN :Failed to join channel. It may require an invite, a password, or you may be banned.\r\n");
            }
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
        std::string response;
        if (banner.size() > 0)
            response += "\n" + banner;
        response += "\nServer info: \n\t*Uptime: " + std::to_string(rounded) +
                               " seconds.\n\t*Compilation: recently\r\n";
        cout << response << endl;
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
        channel &chan = getChannel(message.params[1]);
        chan.userStatusMap[recipient].i = true; //set the user to invited.
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
        if (!userExists(message.params[0]))
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
            connectedUser.userSocket.get()->sendString("NICK " + connectedUser.getName() + " :" + message.params[0] + " is taken!\r\n");
        }
        return 2;
    }

    //allows the user to change their mode
    else if (message.command == "MODE")
    {
        if (message.params[0][0] != '#')
        {
            for (int i = 1; i < message.params.size(); i++)
            {
                std::string mode = message.params[i];
                //switch on the first char, which should be one of the modes
                switch (mode[0])
                {
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
                    connectedUser.userSocket.get()->sendString("Unrecognized mode attribute: " + mode + ".\r\n");
                }
            }
        }
        else
        {
            channel &chan = getChannel(message.params[0]);
            for (int i = 1; i < message.params.size(); i++)
            {
                std::string mode = message.params[i];
                //means that the first user
                if (chan.userStatusMap[connectedUser.getName()].o)
                {
                    if (mode[1] == 'o')
                    {
                        //add mode o to next guy.
                        std::string user = message.params[i + 1];
                        chan.userStatusMap[user].o = plusorminus(mode[0]);
                        i++; //skip next one, obv not a command.
                    }
                    else if (mode[1] == 'p') //checks to see if this guy is an op.
                        chan.p = plusorminus(mode[0]);

                    else if (mode[1] == 's')
                        chan.s = plusorminus(mode[0]);

                    else if (mode[1] == 'i')
                        chan.i = plusorminus(mode[0]);

                    else if (mode[1] == 't')
                        chan.t = plusorminus(mode[0]);

                    else if (mode[1] == 'n')
                        chan.n = plusorminus(mode[0]);

                    else if (mode[1] == 'b')
                    {
                        std::string user = message.params[i + 1];
                        chan.userStatusMap[user].b = plusorminus(mode[0]);
                        chan.b = true;
                        i++; //skip next one, obv not a command.
                    }

                    else if (mode[1] == 'l')
                    {
                        std::string lim = message.params[i + 1];
                        chan.limit = stoi(lim);
                        chan.l = true;
                        i++; //skip next one, obv not a command.
                    }

                    else if (mode[1] == 'k')
                    {
                        std::string key = message.params[i + 1];
                        chan.password = key;
                        i++; //skip next one, obv not a command.
                    }
                }
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
                victim.userSocket.get()->sendString(":" + message.name + " KILL\r\n");
                victim.closeSocket();
                //std::cout << victim.closeSocket();
                return 2;
            }
        }
        else
        {
            connectedUser.userSocket.get()->sendString("You do not have license to kill!\r\n");
        }
        return 2;
    }

    //returns a print out of rules.
    else if (message.command == "RULES")
    {
        std::string retS = "\n***************************\n" + std::string("Be respectful.\n") + "Don't try to break stuff.\n" + "Please.\n" + "Enjoy.\n" + "***************************\r\n";
        connectedUser.userSocket.get()->sendString(retS);
        return 2;
    }

    //returns the version of the server.
    else if (message.command == "VERSION")
    {
        std::string retS = ":server VERSION :Server version 1.0\r\n";
        connectedUser.userSocket.get()->sendString(retS);
        return 2;
    }
    //returns server time.
    else if (message.command == "TIME")
    {
        //follows example from: http://www.cplusplus.com/reference/ctime/strftime/
        time_t now;
        struct tm *timestr;
        char buffer[80];

        time(&now);
        timestr = localtime(&now);
        strftime(buffer, 80, "Server time: %I:%M%p.", timestr);
        std::string timeString(buffer);
        connectedUser.userSocket.get()->sendString(timeString + "\r\n");
        return 2;
    }

    //sets the topic for the channel, or returns the topic for that channel.
    else if (message.command == "TOPIC")
    {
        if (message.params.size() == 2)
        {
            //set the topic.
            channel &chan = getChannel(message.params[0]);
            if (!chan.t || chan.userStatusMap[connectedUser.getName()].o)
                chan.topic = message.params[1];
            return 2;
        }
        else
        {
            try
            {
                std::string retmsg;
                retmsg += ":" + connectedUser.getName() + " TOPIC ";
                channel &chan = getChannel(message.params[0]);
                retmsg += chan.name + " ";
                if (chan.topic.length() > 0)
                {
                    retmsg += ":" + chan.topic + "\r\n";
                }
                else
                {
                    retmsg += ":not set.\r\n";
                }
                connectedUser.userSocket.get()->sendString(retmsg);
                return 2;
            }
            catch (std::string error)
            {
                connectedUser.userSocket.get()->sendString(error + "\r\n");
                return 2;
            }
        }
    }

    //sends a notification to a invitation only server that the user would like an invite.
    //does not notify private servers.
    //does not notify open servers. 
    else if (message.command == "KNOCK")
    {
         try
            {
                std::string recipient = message.params[0];
                //get the channel by name.
                cs457::channel& rcvChannel = getChannel(recipient);
                //send to each member of the channel.
                //if the channel is invitation only
                if(rcvChannel.i)
                {
                    for (cs457::user &rcvUser : rcvChannel.members)
                    {
                        //setting n will also make a server un-knockable. This behavior may not be specified.
                        if (rcvUser.socketActive && !rcvUser.i && (!rcvChannel.n || userInChannel(connectedUser, rcvChannel)))
                        {
                            //does pretty much the same as below. Client should check recipient to see if its a channel then?
                            std::string sendString = ":" + message.name + " KNOCK " + recipient;
                            if(message.params.size() > 1)
                                sendString += " :" + message.params[1] + "\r\n";
                            else 
                                sendString += "\r\n";
                            //sends the recipient the string, although we take out other recipients.
                            //Might not be necessary. Maybe only need to strip channels?
                            rcvUser.userSocket.get()->sendString(sendString);
                        }
                    }
                }
            }
            catch (std::string e)
            {
                //Channel not found. Can just ignore for now.
            }
            return 2;
    }

    //sets the pass of an already authenticated and connected user. Use this to change from default @
    else if (message.command == "PASS")
    {
        connectedUser.setPassword(message.params[0]);
        return 2;
    }

    //Adds the given users to the silence list(even if they don't exist.)
    //Or if no users are given, returns the silence list. 
    else if (message.command == "SILENCE")
    {
        if(message.params.size() < 1)
        {
            //return the list of silenced users
            std::string retStr = "\nList of banned silenced users: ";
            for(auto u : connectedUser.silencedUsers)
            {
                retStr += "\n*" + u.first;
            }
            retStr += "\r\n";
            connectedUser.userSocket.get()->sendString(retStr);
        }
        else
        {
            //silence all users in list. 
            for(std::string user : message.params)
            {
                connectedUser.silencedUsers[user] = true;
            }
        }
        return 2;
    }

    //sets the users "real name"
    else if (message.command == "SETNAME")
    {
        connectedUser.setRealName(message.params[0]);
        return 2;
    }

    //returns ips of the given nicknames
    else if (message.command == "USERIP")
    {
        std::string retStr;
        for (std::string uName : message.params)
        {

            try
            {
                cs457::user &info = getUser(uName);
                retStr += info.getName() + "'s IP address is: ";
                if (info.socketActive)
                    retStr += info.userSocket.get()->getIP() + "\n";
                else
                    retStr += "OFFLINE\n";
            }
            catch (std::string error)
            {
                cout << error << endl;
            }
        }
        if (retStr.length() > 0)
        {
            connectedUser.userSocket.get()->sendString("Found Users:\n" + retStr + "\r\n");
        }
        else
            connectedUser.userSocket.get()->sendString("None of the given users were found!\r\n");
        return 2;
    }

    //return all user and some info about them.
    else if (message.command == "USERS")
    {
        std::string retStr;
        for (auto u : getUsers())
        {
            retStr += "\nUser: " + u.second.getName();
            if (u.second.getRealName().length() > 0)
                retStr += "\n\tRealName: " + u.second.getRealName();
            retStr += "\n\tLevel: " + u.second.getLevel();
            if (u.second.socketActive)
                retStr += "\n\tConnection Status: Online";
            else
                retStr += "\n\tConnection Status: Offline";
        }
        if (retStr.length() > 0)
        {
            connectedUser.userSocket.get()->sendString("\nList of known users:" + retStr + "\r\n");
        }
        else
            connectedUser.userSocket.get()->sendString("No registered users?\r\n");
        return 2;
    }

    //returns information about the given nicknames.
    else if (message.command == "USERHOST")
    {
        std::string retStr;
        for (std::string uName : message.params)
        {

            try
            {
                cs457::user &info = getUser(uName);
                retStr += info.getName() + ": \n";
                if (info.socketActive)
                    retStr += "\t*ONLINE\n";
                else
                    retStr += "\t*OFFLINE\n";
                if (info.getRealName().length() > 0)
                    retStr += "\t*Real Name: " + info.getRealName();
            }
            catch (std::string error)
            {
                cout << error << endl;
            }
        }
        if (retStr.length() > 0)
        {
            connectedUser.userSocket.get()->sendString("\n" + retStr + "\r\n");
        }
        else
            connectedUser.userSocket.get()->sendString("None of the given users were found!\r\n");
        return 2;
    }

    //returns a list of nicknames that have a given realname
    else if (message.command == "WHO")
    {
        std::string retStr;
        for (std::string rName : message.params)
        {
            retStr += "\nThe following users have real name " + rName + ":";
            for (auto u : getUsers())
            {
                if (u.second.getRealName() == rName)
                    retStr += "\n\t*" + u.second.getName();
            }
        }
        if (retStr.length() > 0)
        {
            connectedUser.userSocket.get()->sendString(retStr + "\r\n");
        }
        else
            connectedUser.userSocket.get()->sendString("None of the given users were found!\r\n");
        return 2;
    }

    //Command is not recognized.
    else
    {
        std::cout << "unrecognized command " << message.command << endl
                  << "[SERVER]>";
        return 2;
    }
}

cs457::user &cs457::server::addUserWithSocket(shared_ptr<cs457::tcpUserSocket> clientSocket, bool *cont)
{
    cs457::user connectedUser(clientSocket);
    if (!userExists(connectedUser.getName()))
    {
        //if he doesn't exist, add directly.
        addUser(connectedUser);
        cs457::user &myref = userMap.at(connectedUser.getName());
        return myref;
    }
    else if(!userOnline(connectedUser.getName()))
    {
        bool banned = false;
        for (std::string u : bannedUsers)
        {
            if( u == connectedUser.getName())
                banned = true;
        }
        //already seen, so have to check password.
        cs457::user &returnedUser = getUser(connectedUser.getName());
        if (returnedUser.checkPassword(connectedUser.getPassword()) && !banned)
        {
            returnedUser.setSocket(clientSocket);
            returnedUser.socketActive = true;
            return returnedUser;
        }
        else if (!banned)
        {
            connectedUser.userSocket.get()->sendString("Failed password attempt. Please reconnect.\r\n");
            connectedUser.closeSocket();
            (*cont) = false;
            return returnedUser;
        }
        else 
        {
            connectedUser.userSocket.get()->sendString("You are banned.\r\n");
            connectedUser.closeSocket();
            (*cont) = false;
            return returnedUser; 
        }
    }
    else 
    {
        cs457::user &returnedUser = getUser(connectedUser.getName());
        connectedUser.userSocket.get()->sendString("This nickname is taken. Please try another.\r\n");
        connectedUser.closeSocket();
        (*cont) = false;
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
            if ((!c.i || (c.i && c.userStatusMap[requestingUser.getName()].i)) &&
                (!c.p || (c.p && c.password == pass)) &&
                ((c.members.size() < c.limit) || !c.l) &&
                (!c.b || !c.userStatusMap[requestingUser.getName()].b))
            {
                c.members.push_back(requestingUser);
                return true;
            }
            else
                return false;
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
        if (!c.s)
        {
            std::string attributes = "[";
            if (c.i)
                attributes += "i";
            if (c.p)
                attributes += "p";
            if (c.n)
                attributes += "n";
            attributes += "] ";
            list += attributes;
            if (c.topic.length() > 0)
                list += c.name + ": " + c.topic + "\n";
            else
                list += c.name + "\n";
            if (showUsers)
            {
                for (cs457::user u : c.members)
                {
                    list += ("\t" + u.getName() + "\n");
                }
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