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
#include <string>
#include <iostream>
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
  void closeSocket();
  void setSocket(shared_ptr<cs457::tcpUserSocket> inputSocket);
  shared_ptr<cs457::tcpUserSocket> userSocket;
  std::string getName() const;
  /**
   * This method allows the user on the server to recieve.
   * Make a threaded call to it. 
   */

private:
  std::string username;
  std::string password;
  /**
     * has the function of telling permissions.
     * valid ones are user, channelop, sysop, admin
     * */
  std::string level;
 
};
} // namespace cs457

/***************************************
 * 
 * ohh yeahh
 * https://stackoverflow.com/questions/28957279/how-to-exit-a-blocking-recv-call
 * call shutdown() on that shit to kill it!! unblocks recv!!!
 * /
 */
