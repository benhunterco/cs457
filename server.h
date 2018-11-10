#pragma once

#include "user.h"
#include <string>
#include <map>
#include <vector>
#include <istream>
#include <fstream> 
#include "Parsing.h"
#include <time.h>
/** This class builds a representation of the state of the server.
 * In the future, it will be able to load in the state from files and write out to files.
 * For now, it simply keeps track of the connected users in userMap, 
 */

namespace cs457
{

//These are the flags a channel member has.
//So if some of them are true, then he has those.
//others will be set later. 
struct channelModes
{
    bool o; //whether the member is an op
    bool i; //whether the member is invited.
    bool b; //whether the member is banned.
};

struct channel
{
    std::string name;
    std::string password;
    std::vector<cs457::user> members;
    std::string topic;
    std::map<std::string, channelModes> userStatusMap;
    int limit;
    //channel portion of modes
    bool p = false; //whether the channel is private. Here means it is static.
    bool s = false; //whether channel is secret, won't show in list.
    bool i = false; //requires invite, but can bee seen. Unless s is set.
    bool t = false; //topic settable only by some.
    bool n = false; //deny non member messages
    bool l = false; //user limit.
    bool b = false; //whether there is a ban mask. 
    bool k = false; //sets the password, basically whether there is a pass.
    
}; 
class server
{
  public:
    bool addUser(cs457::user);
    bool removeUser(cs457::user);
    cs457::user& addUserWithSocket(shared_ptr<cs457::tcpUserSocket>, bool*);
    bool addChannel(cs457::user, std::string channelName);
    bool addUserToChannel(cs457::user&, std::string channelName, std::string pass = "@");
    bool userInChannel(cs457::user&, cs457::channel&);
    //std::map<std::string, cs457::user> getUserMap();
    std::vector<channel> getChannels();//isnt implemented?
    cs457::channel& getChannel(std::string& channelName);
    std::map<std::string, cs457::user> getUsers();
    cs457::user& getUser(std::string);
    /*Parses and evaluates the command given. 
      lets see if this is a good way of doing things
      */
    std::vector<std::string> bannedUsers;
    int command(std::string, cs457::user&);
    std::string dbPath = "db/";
    std::string listChannels(bool showUsers = false);
    time_t startTime;
    bool userExists(std::string);
    bool userOnline(std::string);

    bool writeUsers();
    bool writeBans();
    bool writeChannels();

    bool readUsers();
    void addUserFromFile(std::string);
    bool readBans();
    bool readChannels();

    std::string banner;
    bool readBanner();

  private:
    std::map<std::string, cs457::user> userMap;
    std::vector<channel> channels;
};
} // namespace cs457