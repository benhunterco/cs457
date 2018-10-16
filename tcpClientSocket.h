#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <memory.h>
#include <arpa/inet.h>
#include <mutex>

/*
make this some sort of usefull wrapper class.
Ideally, you construct it and just give it a port.
Then, you can use operator << to send stuff across the socket!! 
It could even take objects!.
And maybe do operator >> ??? That could be cool. 
Anyway, I am way less scared now. 
*/

namespace cs457
{

using namespace std;

class tcpClientSocket
{

  public:
    tcpClientSocket(int port, string serverAddress);
    tuple<string,ssize_t> recvString(int bufferSize=4096,bool useMutex = true);
    size_t sendString(string message, bool mutex);

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