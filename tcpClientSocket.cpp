#include "tcpClientSocket.h"
#include <iostream>
#include <istream>

cs457::tcpClientSocket::tcpClientSocket(int port, std::string serverAddress)
{
    tcpClientSocket::port = port;
    tcpClientSocket::serverAddress = serverAddress;

    //now, do the socket initializy stuff.
    //First, try to create the socket. 
    if((tcpClientSocket::socketID = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        std::cerr << "Error in creating Socket!\n";
    }

    //Then clear out serv_addr
    memset(&serv_addr, '0', sizeof(serv_addr)); 

    //Set Protocol of serv_addr to ipv4
    serv_addr.sin_family = AF_INET; 

    //Set port to desired outbound port, I think.
    serv_addr.sin_port = htons(tcpClientSocket::port);

    //Set the outbound address based on our string. Must change our c++ string into char*
    if(inet_pton(AF_INET, tcpClientSocket::serverAddress.c_str(), &serv_addr.sin_addr)<=0)  
    { 
        std::cerr << "\nInvalid address/ Address not supported \n"; 
    } 

    //Make connection.
    if (connect(socketID, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        std::cerr << "\nConnection Failed \n"; 
    } 
}

std::string cs457::tcpClientSocket::sendString(std::string message)
{
    message += "\n";
    send(socketID, message.c_str(), sizeof(message.c_str()), 0);
    std::cout << "Client is sending: " << message.c_str();
    return "sentsuccs";
}