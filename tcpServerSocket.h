#pragma once

#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <strings.h>
#include <string> 
#include <tuple> 
#include <unistd.h>
#include <memory> 


//class forwards 
namespace cs457
{
    class tcpUserSocket; 
    
}

namespace cs457
{
    using namespace std; 
    class tcpServerSocket
    {
    public:
        tcpServerSocket(uint portNumber); 
        tcpServerSocket(string networkAddress, uint portNumber);
        int bindSocket();
        int listenSocket();
        tuple<shared_ptr<cs457::tcpUserSocket>,int> acceptSocket();
      
    private: 
        void init();
        void setSocketOptions();
        uint port; 
        string address; 
        
     
        //
        int serverSocket;
        
        struct sockaddr_in serverAddress; 
        struct in_addr addr;
        
    };


}

