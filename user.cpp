#include "user.h"
#include "Parsing.h"
#include <string>
#include <iostream>

int cs457::user::closeSocket()
{
    int retint = -100;
    if (cs457::user::userSocket)
    {
        retint = userSocket->closeSocket();
        socketActive = false;
    }
    return retint;
}

void cs457::user::setSocket(shared_ptr<cs457::tcpUserSocket> inputSocket)
{
    socketActive = true;
    userSocket = inputSocket;
}


//c++ is weird, watch the defaults here. 
cs457::user::user(std::string uname, std::string pword /*= "@"*/,
                  std::string lvl /*= "user"*/, std::string banStatus, shared_ptr<cs457::tcpUserSocket> inputSocket /*= nullptr*/)
{
    username = uname;
    password = pword;
    level = lvl;
    if (inputSocket)
    {
        userSocket = inputSocket;
        socketActive = true;
    }
    else
    {
        userSocket = nullptr;
        socketActive = false;
    }
    if (banStatus == "true")
        banned = true;
    else
        banned = false;
}

//This is the one I'm using right now for adding as the connect. 
cs457::user::user(shared_ptr<cs457::tcpUserSocket> inputSocket)
{
    userSocket = inputSocket;
    socketActive = true;
    std::string msg; int val;
    //recieve the first command from the client to attempt to build a new user.
    tie(msg, val) = userSocket.get()->recvString();
    if(val > 0){
        //append the crlf thing???
        //ohh yeah probably do this clientside!!!!TODO
        Parsing::IRC_message message(msg);
        if(message.command == "PASS"){
            //New user registering on first connection.
            username = message.name;
            //set the new hypothetical users password
            password = message.params[0];
        }
        else
        {
            //no password given. 
            username = message.name;
        }
    }
    else{
        /*Something went wrong!!!*/
    }
    realName = "";
    awayMessage = "I, " + username + " am not here right now.";
    
}

std::string cs457::user::getName()const
{
    return username;
}

void cs457::user::setName(std::string name)
{
    username = name;
}

void cs457::user::setAwayMessage(std::string newMessage)
{
    awayMessage = newMessage;
    return;
}

std::string cs457::user::toString()
{
    //helpful for the file writing.
    std::string ban;
    if(banned)
        ban = "true";
    else
        ban = "false";
    std::string retStr = username + " " + password + " " + level + " " + ban;
    return retStr;
}

std::string cs457::user::getAwayMessage(){
    return awayMessage;
}

void cs457::user::setLevel(std::string newLevel)
{
    level = newLevel;
}
  
std::string cs457::user::getLevel()
{
    return level;
}

std::string cs457::user::getPassword()
{
    return password;
}

bool cs457::user::checkPassword(std::string match)
{
    return password == match;
}

void cs457::user::setPassword(std::string pass)
{
    password = pass;
}

void cs457::user::setRealName(std::string name)
{
    realName = name;
}

std::string cs457::user::getRealName()
{
    return realName;
}
/*
std::ostream& operator<<(std::ostream& os, const cs457::user us)
{
    os << us.getName();
    return os;
}*/

