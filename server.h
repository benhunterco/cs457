#pragma once

#include "user.h"
#include <string>
#include <map>
#include <vector>
#include <istream>
#include <fstream> 
#include "Parsing.h"
#include <time.h>

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

//this is the struct that represents a channel.
//fairly straightforward
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

/** This class builds a representation of the state of the server.
 *  The administrator can use file io to write out and read in to cerain values.
 *  In addtion to keeping track of state, it also handles all of the commands the server can recieve
 */
class server
{
  public:
    //adds a user to the map. Does some funky reference stuff so that the map contains THE user, keeps it in memory
    bool addUser(cs457::user);
    //removes a user from that map
    bool removeUser(cs457::user);
    //Used on the very initial connection when the only info about a user is their socket.
    cs457::user& addUserWithSocket(shared_ptr<cs457::tcpUserSocket>, bool*);
    //creates and adds a channel with a creator and channel name. like on join
    bool addChannel(cs457::user, std::string channelName);
    //creates and adds a channel based on password and name, used on file read.
    bool addChannel(std::string, std::string);
    //adds a user to a channel if the password is correct and returns accordingly
    bool addUserToChannel(cs457::user&, std::string channelName, std::string pass = "@");
    //checks if a user is in a given channel and returns accordingly
    bool userInChannel(cs457::user&, cs457::channel&);


    std::vector<channel> getChannels();//isnt implemented?
    //gets a channel reference by name. Can be changed directly because its reference
    cs457::channel& getChannel(std::string& channelName);
    //gets the map of users in the channel. Helpful for iteration
    std::map<std::string, cs457::user> getUsers();
    //gets a user by string, throws if now found.
    cs457::user& getUser(std::string);
   
    //keeps track of banned users. 
    std::vector<std::string> bannedUsers;

    //interprets commands of a string and a user
    int command(std::string, cs457::user&);
    //just the default of the database
    std::string dbPath = "db/";
    //returns a list of users for both list and admin. 
    //members are optional
    std::string listChannels(bool showUsers = false);
    //tracks uptime
    time_t startTime;

    //protect user queries based on whether they exist and whether their online.
    bool userExists(std::string);
    bool userOnline(std::string);

    //file writing methods
    bool writeUsers();
    bool writeBans();
    bool writeChannels();

    //file reading methods
    bool readUsers();
    void addUserFromFile(std::string);
    void addChannelFromFile(std::string);
    bool readBans();
    bool readChannels();

    std::string banner;
    bool readBanner();

  private:
    //stateful data.
    std::map<std::string, cs457::user> userMap;
    std::vector<channel> channels;
};
} // namespace cs457