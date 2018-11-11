/*This is a class that represents users.
Each user is essentially an active, or inactive connection. 
They must have :
*username
*password <-empty is @
*level
*banned <-true/false

Each connected user also has a connection cclient thread.
We can kill these with unique identifier map.
Hmm

For the server's use.
*/
#pragma once
#include <string>
#include <iostream>
#include <map>
#include "tcpUserSocket.h"

namespace cs457
{
class user
{
public:

  //bools that hold status
  bool banned;
  bool socketActive;
  
  //these constructors provide different uses. the socket one handles an incoming new connection.
  //the complicated one handles creating users from a file or in different cases.
  user(std::string uname, std::string password = "@", std::string level = "user", std::string banStatus = "false",
       shared_ptr<cs457::tcpUserSocket> inputSocket = nullptr);
  user(shared_ptr<cs457::tcpUserSocket> inputSocket);

  //closes the users socket.
  int closeSocket();
  //sets it to the given socket.
  void setSocket(shared_ptr<cs457::tcpUserSocket> inputSocket);
  
  //manipulators for away message.
  void setAwayMessage(std::string);
  std::string getAwayMessage();

  //socket with which to send communication.
  shared_ptr<cs457::tcpUserSocket> userSocket;

  //name and level manipulators
  std::string getName() const;
  void setName(std::string);
  void setLevel(std::string level);
  std::string getLevel();
 

  //modes section
  bool i = false; //whether to not recieve privmsg.
  bool s = true;  //recieves notice command
  bool w = false; //recieves wallops
  //note. Operator flag is not added, because that is treated through the oper command only. <-instead its a level.
  
  //password manipulators.
  std::string getPassword();
  bool checkPassword(std::string);
  void setPassword(std::string);

  //same for realname
  std::string getRealName();
  void setRealName(std::string);

  //simple map for silencing users. any user can be added, they don't need to exist.
  std::map<std::string, bool> silencedUsers;

  std::string toString();

private:

  //stateful data.
  std::string username;
  std::string password;
  /**
     * has the function of telling permissions.
     * valid ones are user, channelop, sysop, admin
     * */
  std::string level = "user";
  /**
   * Is set by the /away command.
   * gets returned when hit by privmsg. Maybe something else too?
   */
  std::string awayMessage;
  //this is the real name. 
  std::string realName;
};
} // namespace cs457

