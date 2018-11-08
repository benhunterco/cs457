#include "tcpClientSocket.h"
#include <iostream>
#include <istream>

cs457::tcpClientSocket::tcpClientSocket(int port, std::string serverAddress)
{
    tcpClientSocket::port = port;
    tcpClientSocket::serverAddress = serverAddress;

    //now, do the socket initializy stuff.
    //First, try to create the socket.
    if ((tcpClientSocket::socketID = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Error in creating Socket!\n";
    }

    //Then clear out serv_addr
    memset(&serv_addr, '0', sizeof(serv_addr));

    //Set Protocol of serv_addr to ipv4
    serv_addr.sin_family = AF_INET;

    //Set port to desired outbound port, I think.
    serv_addr.sin_port = htons(tcpClientSocket::port);

    //Set the outbound address based on our string. Must change our c++ string into char*
    //change to getaddrinfo from here http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    //and here https://gist.github.com/jirihnidek/bf7a2363e480491da72301b228b35d5d
    if (inet_pton(AF_INET, tcpClientSocket::serverAddress.c_str(), &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "\nInvalid address/ Address not supported \n";
    }

    //Make connection.
    if (connect(socketID, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "\nConnection Failed \n";
    }
}

size_t cs457::tcpClientSocket::sendString(const std::string& message, bool useMutex)
{
    //add new line from command prompt, not here.
    //message += "\n";
    size_t rval;
    if(debug)
        std::cout << "\n[DEBUG] Client is sending: " << message.c_str()<<std::endl;
    const char* cstring = message.c_str();
    if (useMutex)
    {
        lock_guard<mutex> lock(sendMutex);
        rval = send(socketID, cstring, strlen(cstring), 0);
    }
    else
    {
        rval = send(socketID, cstring, strlen(cstring), 0);
    }

    return rval;
}

std::tuple<std::string,ssize_t> cs457::tcpClientSocket::recvString(int bufferSize, bool useMutex)
{
    char stringBuffer[bufferSize]; 
    memset(stringBuffer, 0, sizeof(stringBuffer));    //change made here. Zeros out buffer.

    ssize_t recvMsgSize;

    if (useMutex)
    {
        lock_guard<mutex> lock(recieveMutex);
        recvMsgSize = recv(socketID, stringBuffer, bufferSize, 0); 
    }    
    else
    {
        recvMsgSize = recv(socketID, stringBuffer, bufferSize, 0); 
    }
    
    
   
    return make_tuple(string(stringBuffer),recvMsgSize);     
};