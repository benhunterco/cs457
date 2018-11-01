#include "client.h"

void cs457::client::send(std::string msg){
    //this should help to clean up the commands a littl bit. Automatically appends stuff.
    sock->sendString(":" + username + " " + msg + "\r\n", true);
}