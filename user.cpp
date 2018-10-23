#include "user.h"
#include <string>

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
