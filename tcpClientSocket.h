#pragma once

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <memory.h>
#include <arpa/inet.h>
#include <mutex>



namespace cs457
{

using namespace std;

/**
 * uses a combination of stuff from the class skeleton used on the server and online documentation
 * similar except for a few variable names.
 * also adds a few debug prints
 */
class tcpClientSocket
{

  public:
    tcpClientSocket(int port, string serverAddress);
    tuple<string,ssize_t> recvString(int bufferSize=4096,bool useMutex = true);
    size_t sendString(const string& message, bool mutex);
    bool debug = false;

  private:
    struct sockaddr_in address;
    int socketID;
    int port;
    string serverAddress;
    struct sockaddr_in serv_addr;
    mutex sendMutex;
    mutex recieveMutex;
};
} 