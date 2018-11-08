#pragma once

#include <string>
#include <iostream>
#include "tcpClientSocket.h"
#include "Parsing.h"
namespace cs457
{
class client
{
  public:
    std::string hostname = "127.0.0.1";
    std::string username = "bobby";
    std::string password = "@";
    int serverport = 2000;
    std::string configFile = "";
    std::string testFile = "";
    std::string logFile = "";
    tcpClientSocket* sock;
    size_t send(std::string msg);
    size_t registerUser();
    //Recieves the given input and decides what to do with it. 
    //some classes can just be sent, some need input on this end.
    int command(std::string command);
    //if we recieve privmsg->show stuff.
    //if we recieve ping->pong.
    int rcvCommand();

  private:
};
} // namespace cs457