#include "user.h"
#include "Parsing.h"
#include <string>
#include <iostream>

void cs457::user::closeSocket()
{
    if (cs457::user::userSocket)
    {
        cs457::user::userSocket->closeSocket();
        cs457::user::socketActive = false;
    }
}

void cs457::user::setSocket(shared_ptr<cs457::tcpUserSocket> inputSocket)
{
    cs457::user::userSocket = inputSocket;
}


//c++ is weird, watch the defaults here. 
cs457::user::user(std::string uname, std::string pword /*= "@"*/,
                  std::string lvl /*= "user"*/, shared_ptr<cs457::tcpUserSocket> inputSocket /*= nullptr*/)
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
}

cs457::user::user(shared_ptr<cs457::tcpUserSocket> inputSocket)
{
    userSocket = inputSocket;
    std::string msg; int val;
    tie(msg, val) = userSocket.get()->recvString();
    if(val > 0){
        //append the crlf thing???
        msg += "\r\n";
        Parsing::IRC_message message(msg);
        if(message.command == "NICK"){
            //New user registering on first connection.
            username = message.params[0];
        }
    }
    else{
        /*Something went wrong!!!*/
    }
    
}

std::string cs457::user::getName()const
{
    return username;
}

/*
std::ostream& operator<<(std::ostream& os, const cs457::user us)
{
    os << us.getName();
    return os;
}*/

