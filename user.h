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
#include "tcpUserSocket.h"

namespace cs457
{
class user
{
  public:
    std::string username;
    std::string password;
    std::string level;
    bool banned;
    shared_ptr<cs457::tcpUserSocket> userSocket;
};
} // namespace cs457




/***************************************
 * 
 * ohh yeahh
 * https://stackoverflow.com/questions/28957279/how-to-exit-a-blocking-recv-call
 * call shutdown() on that shit to kill it!! unblocks recv!!!
 * /
 */

