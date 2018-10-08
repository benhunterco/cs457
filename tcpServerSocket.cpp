#include <netinet/in.h> 
#include <arpa/inet.h>
#include "tcpServerSocket.h"
#include "tcpUserSocket.h"


using namespace std; 

cs457::tcpServerSocket::tcpServerSocket(uint portNumber): port(portNumber), address("")
{
    init(); 
    setSocketOptions();
};     

cs457::tcpServerSocket::tcpServerSocket(string networkAddress, uint portNumber): address(networkAddress), port(portNumber)
{
    init(); 
    setSocketOptions();
};

int cs457::tcpServerSocket::bindSocket()
{
    return bind(serverSocket,(struct sockaddr *)&serverAddress,sizeof(serverAddress));    
}
        
int cs457::tcpServerSocket::listenSocket()
{
    return listen(serverSocket,14);
}

tuple<shared_ptr<cs457::tcpUserSocket>,int> cs457::tcpServerSocket::acceptSocket()
{
    shared_ptr<cs457::tcpUserSocket> userSocket = make_shared<tcpUserSocket>(); 
    socklen_t len = userSocket.get()->getLenghtPointer();
    int client_fd = accept(serverSocket,(struct sockaddr *)userSocket.get()->getAddressPointer(),&len);
    userSocket.get()->setSocket(client_fd); 
    //get ip and port 
    // you can always get it like this: getsockname(sockfd, (struct sockaddr *) &my_addr, &len);
    //but you already have it
    char userIPv4[16]; 
    sockaddr_in * userAddress =  (sockaddr_in *) userSocket.get()->getAddressPointer();
    inet_ntop(AF_INET, &(userAddress->sin_addr), userIPv4, INET_ADDRSTRLEN);
    
    auto  clientPort = ntohs(userAddress->sin_port);

    userSocket.get()->setUserInfoIPv4(string(userIPv4),clientPort); 


    return make_tuple(userSocket,client_fd); 
}

 void cs457::tcpServerSocket::init()
 {
    serverSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    const char * cstr = address.c_str();
    int val =0;  
    bzero(&serverAddress,sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    if (address == "")
        addr.s_addr =  htonl(INADDR_ANY);
    else
    {
        val = inet_aton(cstr,&addr);
        serverAddress.sin_addr = addr;
        string addresscpy(inet_ntoa(addr));
        address = addresscpy;  
    }

    serverAddress.sin_port = htons(port);
}

void cs457::tcpServerSocket::setSocketOptions()
{
    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, 
	                (const void *)&optval , sizeof(int));
}