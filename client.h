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
    void send(std::string msg);

  private:
};
} // namespace cs457