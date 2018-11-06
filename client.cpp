#include "client.h"

size_t cs457::client::send(std::string msg)
{
    //this should help to clean up the commands a littl bit. Automatically appends stuff.
    return sock->sendString(":" + username + " " + msg + "\r\n", true);
}

size_t cs457::client::registerUser()
{
    /**send appropriate registration details
     */
    std::string registration = "NICK " + username + "\r\n";
    return send(registration);
    //possibly do passwordy stuff here.
}

int cs457::client::command(std::string command)
{
    int retVal = 1;
    if(command[0] == '/')
    {
        send(command.substr(1, command.length() - 1)); //sends string without slash.
        if(command.substr(1,4) == "QUIT") //un continues.
            retVal = 0;
    }
    else{
        //send to active channel.
    }
    return retVal;
}