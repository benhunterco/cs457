#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <tuple>
#include <thread>
#include <vector>
#include <memory>
#include <map>
#include "tcpUserSocket.h"
#include "tcpServerSocket.h"
#include "Parsing.h"
#include "user.h"

using namespace std;

bool ready = true;

//In the future, use this to silence or enable printing.
bool verbose = true;

//add user map parameter.
int cclient(shared_ptr<cs457::tcpUserSocket> clientSocket, int id)
{

    cout << "Waiting for message from Client Thread" << id << std::endl;
    /**
     * here the client should send in their pass and user info. 
     * Then we can create a user for them.
     * We'll use the format of IRC messages for this I think. 
     */
    string msg;
    ssize_t val;
    bool cont = true;
    while (cont)
    {
        tie(msg, val) = clientSocket.get()->recvString();

        //from man(recv), a return value of 0 indicates "stream socket peer has performed orderly shutdown".
        if (val == 0)
        {
            //Client has disconnected
            //or possibly, socket was killed elsewhere!
            cout << "Client " << id <<" Disconnected\n";
            cont = false;
            break;
        }
        if (msg.substr(0, 4) == "EXIT")
            cont = false;

        cout << "[SERVER] The client is sending message " << msg << " -- With value return = " << val << endl;
        string s = "[SERVER REPLY] The client is sending message:" + msg + "\n";
        thread childT1(&cs457::tcpUserSocket::sendString, clientSocket.get(), s, true);
        //thread childT2(&cs457::tcpUserSocket::sendString,clientSocket.get(),msg,true);
        //thread childT3(&cs457::tcpUserSocket::sendString,clientSocket.get(),"\n",true);

        childT1.join();
        //childT2.join();
        //childT3.join();
        //clientSocket.get()->sendString(msg);
        //clientSocket.get()->sendString("\n");
        if (msg.substr(0, 6) == "SERVER")
        {
            thread childTExit(&cs457::tcpUserSocket::sendString, clientSocket.get(), "GOODBYE EVERYONE", false);
            thread childTExit2(&cs457::tcpUserSocket::sendString, clientSocket.get(), "\n", false);
            ready = false;
            cont = false;
            childTExit.join();
            childTExit2.join();
        }
        else
        {
            cout << "waiting for another message" << endl;
        }
    }

    clientSocket.get()->sendString("goodbye");

    clientSocket.get()->closeSocket();
    //remove client from map here.
    return 1;
}

//This method runs on its own thread. Commands are concurent with message receiving. Maybe could put in a verbose flag?
void adminCommands(map<string, shared_ptr<cs457::tcpUserSocket>> *uMap)
{
    string command;
    bool continueAdmin = true;
    while (continueAdmin)
    {
        cout << "[SERVER]>";
        getline(cin, command);
        //adds returns stuff that the parser wants, not automatic with getline.
        Parsing::IRC_message message(command + "\r\n");
        //enum of commands?
        if (message.command == string("USERS"))
        {
            for (auto u : *uMap)
            {
                //should print out the keys in uMap.
                cout << "Key: " << u.first << endl;
                cout << "Socket: " << u.second->getSocket() << endl;
                cout << "UniqueID: " << u.second->getUniqueIdentifier() << endl;
            }
        }
        else if (message.command == string("PING"))
        {
            if (message.params.size() > 1)
                cout << "[SERVER]> more than one server on ping not yet supported.\n";

            else if (message.params.size() < 1)
                cout << "[SERVER]> address needed for ping\n";

            else
                cout << "[SERVER]> Sending ping to " << message.params[0] << endl;
        }
        else if (message.command == string("EXIT"))
        {
            continueAdmin = false;
            //Kill all threads and disconect clients here!
        }
        else
        {
            cout << "[SERVER]> Did not recognize: " << command << endl;
        }
    }
}

int port = 2000;
string configFile = "chatserver.conf";
string db = "db/";

int main(int argc, char *argv[])
{
    opterr = 0;
    char c = ' ';
    while ((c = getopt(argc, argv, "p:c:d:")) != -1)
        switch (c)
        {
        case 'p':
            port = atoi(optarg);
            if (port == 0)
            {
                std::cerr << "Incorrect port number. Please enter an integer\n";
                return 1;
            }
            break;
        case 'c':
            configFile = optarg;
            break;
        case 'd':
            db = optarg;
        case '?':
            std::cerr << "Incorrect usage. Options are -port portnumber -c configfile -d /path/to/db \n";
            return 1;
        default:
            abort();
        }

    cout << "Initializing Socket on port " << port << std::endl;
    cs457::tcpServerSocket mysocket(port);
    cout << "Binding Socket" << std::endl;
    mysocket.bindSocket();
    cout << "Listening Socket" << std::endl;
    mysocket.listenSocket();
    cout << "Waiting to Accept Socket" << std::endl;
    int id = 0;
    //this vector will keep track of threads for our listening.
    vector<unique_ptr<thread>> threadList;
    //This map, with key of nickname will keep track of connected clients
    map<string, cs457::user> *userMap = new map<string, cs457::user>;
    cout << "Starting administration thread here??? \n";
    thread adminThread(adminCommands, userMap);
    while (ready)
    {
        shared_ptr<cs457::tcpUserSocket> clientSocket;
        int val;
        tie(clientSocket, val) = mysocket.acceptSocket();
        cout << "value for accept is " << val << std::endl;
        cout << "Socket Accepted" << std::endl;
        unique_ptr<thread> t = make_unique<thread>(cclient, clientSocket, id);
        threadList.push_back(std::move(t));

        id++; //not the best way to go about it.
              // threadList.push_back(t);
    }

    //somehow get users associated?.
    for (auto &t : threadList)
    {
        t.get()->join();
    }
    adminThread.join();
    delete (userMap);
    cout << "Server is shutting down after one client" << endl;
    return 0;
}