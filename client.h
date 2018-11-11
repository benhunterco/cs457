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
    //hostname to connect to
    std::string hostname = "127.0.0.1";
    //username sent in messages. Overwritten by -u
    std::string username = "bobby";
    //Password sent in registration
    //even if it is not yet set, we still send @.
    //this means all users are registered by default.
    std::string password = "@";
    //port on which to open up the socket. 
    int serverport = 2000;
    //if set in command line arguments, overwrites all other relevant params.
    std::string configFile;

    //the following are not yet used.
    std::string testFile = "";
    std::string logFile = "";

    //this is a pointer in order to be usable across threads.
    //shared pointer caused a mess of compiler errors
    tcpClientSocket* sock;

    //another abstraction on the send method of the socket.
    size_t send(std::string msg);
    //pass off the necessary information to register a client.
    //Including their username and other stuff.
    size_t registerUser();

    //Recieves the given input and decides what to do with it. 
    //some classes can just be sent, some need input on this end.
    //HELP for example, doesn't need to contact the server, so we just output directly.
    //Also provides some rudimentary parameter checking. Some stuff is safe, some isn't yet. 
    int command(std::string command);
    
    //This handles stuff we get from the server.
    //some messages are interpreted. PRIVMSG is broken apart to show user and other stuff
    //some is just piped into output.
    int rcvCommand();

    //whether to print out debug statements
    bool debug = false;

    //doesnt do anything even if true. yet.
    bool log = false;

    //These are used to maintain which user we sent to most recently
    //allows the user to just type without the privmsg command
    std::string directUserOrChannel;
    bool sendDirect = false;

  private:
};
} // namespace cs457