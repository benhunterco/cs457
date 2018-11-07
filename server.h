#pragma once

#include "user.h"
#include <string>
#include <map>
#include <vector>
#include <istream>
#include "Parsing.h"
#include <time.h>
/** This class builds a representation of the state of the server.
 * In the future, it will be able to load in the state from files and write out to files.
 * For now, it simply keeps track of the connected users in userMap, 
 */

namespace cs457
{
struct channel
{
    std::string name;
    std::string password;
    std::vector<cs457::user> members;
    
}; 
class server
{
  public:
    bool addUser(cs457::user);
    cs457::user& addUserWithSocket(shared_ptr<cs457::tcpUserSocket>);
    bool addChannel(cs457::user, std::string channelName);
    bool addUserToChannel(cs457::user&, std::string channelName, std::string pass = "@");
    //std::map<std::string, cs457::user> getUserMap();
    std::vector<channel> getChannels();//isnt implemented?
    cs457::channel& getChannel(std::string& channelName);
    std::map<std::string, cs457::user> getUsers();
    cs457::user& getUser(std::string);
    /*Parses and evaluates the command given. 
      lets see if this is a good way of doing things
      */
    bool command(std::string, cs457::user&);
    std::string listChannels(bool showUsers = false);
    time_t startTime;

  private:
    std::map<std::string, cs457::user> userMap;
    std::vector<channel> channels;
};
} // namespace cs457