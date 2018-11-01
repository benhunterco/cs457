#pragma once

#include "user.h"
#include <string>
#include <map>
#include <vector>
#include "Parsing.h"
/** This class builds a representation of the state of the server.
 * In the future, it will be able to load in the state from files and write out to files.
 * In the future, it will be able to to commands.
 * For now, it simply keeps track of the connected users in userMap, 
 * and the channels as vectors of strings. Vect[0] is the channel name, and users are added to it.
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
    std::vector<channel> getChannels();
    std::map<std::string, cs457::user> getUsers();
    cs457::user& getUser(std::string);
    /*Parses and evaluates the command given. 
      lets see if this is a good way of doing things
      */
    bool command(std::string, cs457::user&);
    std::string listChannels(bool showUsers = false);

  private:
    std::map<std::string, cs457::user> userMap;
    std::vector<channel> channels;
};
} // namespace cs457