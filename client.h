#pragma once

#include <string>
#include "tcpClientSocket.h"
namespace cs457
{
class client
{
  public:
    std::string hostname = "127.0.0.1";
    std::string username = "bobby";
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

  private:
};
} // namespace cs457