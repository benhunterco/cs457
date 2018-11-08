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
  bool banned;
  bool socketActive;
  
  user(std::string uname, std::string password = "@", std::string level = "user",
       shared_ptr<cs457::tcpUserSocket> inputSocket = nullptr);
  user(shared_ptr<cs457::tcpUserSocket> inputSocket);
  int closeSocket();
  void setSocket(shared_ptr<cs457::tcpUserSocket> inputSocket);
  void setAwayMessage(std::string);
  std::string getAwayMessage();
  shared_ptr<cs457::tcpUserSocket> userSocket;
  std::string getName() const;
  void setName(std::string);
  void setLevel(std::string level);
  std::string getLevel();
  /**
   * This method allows the user on the server to recieve.
   * Make a threaded call to it. 
   */
  //modes section
  bool i = false; //whether to not recieve privmsg.
  bool s = true;  //recieves notice command
  bool w = false; //recieves wallops
  //note. Operator flag is not added, because that is treated through the oper command only.
  std::string getPassword();
  bool checkPassword(std::string);
  void setPassword(std::string);

  std::string getRealName();
  void setRealName(std::string);

  std::map<std::string, bool> silencedUsers;

private:
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
  //this is the real name. Not that It gets used anywhere at all....
  std::string realName;
};
} // namespace cs457

/***************************************
 * 
 * ohh yeahh
 * https://stackoverflow.com/questions/28957279/how-to-exit-a-blocking-recv-call
 * call shutdown() on that shit to kill it!! unblocks recv!!!
 * /
 */
