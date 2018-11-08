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
#include "server.h"

using namespace std;

bool ready = true;

//In the future, use this to silence or enable printing.
bool verbose = true;
bool debug = false;
//add user map parameter.
int cclient(shared_ptr<cs457::tcpUserSocket> clientSocket, int id, cs457::server *myServer)
{

    bool cont = true; //two means go, 1 means leave, 0 means die.
    //first, we register the user. Could be its own method?
    cs457::user &connectedUser = myServer->addUserWithSocket(clientSocket, &cont);
    if(cont){
        cout << "[SERVER] Connected user: " << connectedUser.getName() << std::endl;
        cout << "[SERVER] Waiting for message from Client Thread" << id << std::endl;
    }
    else{
        cout << "\n[SERVER] Failed password from: " << connectedUser.getName() << std::endl;
    }
    /**
     * here the client should send in their pass and user info. 
     * Then we can create a user for them.
     * We'll use the format of IRC messages for this I think. 
     */
    string msg;
    ssize_t val;
    while (cont)
    {
        tie(msg, val) = clientSocket.get()->recvString();

        //from man(recv), a return value of 0 indicates "stream socket peer has performed orderly shutdown".
        if (val <= 0)
        {
            //Client has disconnected
            //or possibly, socket was killed elsewhere!
            cout << "[SERVER] Client " << id << " Disconnected\n";
            cont = false;
            break;
        }
        //call server command
        //return value could be boolean, indicates whether to continue.
        int ret = myServer->command(msg, connectedUser);
        if (debug)
        {
            cout << "[DEBUG] The client is sending message " << msg << " -- With value return = " << val << endl;
            string s = "[DEBUG] The client is sending message:" + msg + "\n";
            thread childT1(&cs457::tcpUserSocket::sendString, clientSocket.get(), s, true);
            childT1.join();
        }
        else
        {
            cout << "[SERVER] waiting for another message \n[SERVER]>" << std::flush;
        }
       
        //deterimine whether we should continue.
        switch (ret)
        {
            case 0:
                cont = false;
                ready = false;
                break;
            case 1:
                cont = false;
                break;
            case 2: 
                cont = true;
        }
        

    }

    //remove client from map here.
    return 1;
}

//This method runs on its own thread. Commands are concurent with message receiving. Maybe could put in a verbose flag?
void adminCommands(cs457::server *myServer)
{
    string command;
    bool continueAdmin = true;
    while (continueAdmin && ready)
    {
        cout << "[SERVER]>";
        getline(cin, command);
        //adds returns stuff that the parser wants, not automatic with getline.
        Parsing::IRC_message message(command + "\r\n");
        //enum of commands?
        if (message.command == string("USERS"))
        {
            for (auto u : myServer->getUsers())
            {
                //should print out the keys in uMap.
                cout << "Key: " << u.first << endl;
                cout << "Socket: " << u.second.getName() << endl;
                cout << "UniqueID: " << u.second.userSocket.get()->getUniqueIdentifier() << endl;
                cout << "Connected: " << u.second.socketActive << endl;
                cout << "Level: " << u.second.getLevel() << endl;
            }
        }
        else if (message.command == string("PING"))
        {
            if (message.params.size() > 1)
                cout << "[SERVER]> more than one server on ping not yet supported.\n";

            else if (message.params.size() < 1)
                cout << "[SERVER]> address needed for ping\n";

            else
            {
                cout << "[SERVER]> Sending ping to " << message.params[0] << endl;
                cs457::user pingUser = myServer->getUser(message.params[0]);
                pingUser.userSocket.get()->sendString("PING", true);
            }
        }
        else if (message.command == string("EXIT"))
        {
            continueAdmin = false;
            //Kill all threads and disconect clients here!
        }
        else if (message.command == "CHANNELS")
        {
            std::string channels = myServer->listChannels(/*showusers = */ true);
            cout << "Found channels:\n"
                 << channels;
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
    //create the server object and get its start time. 
    cs457::server myServer;
    time(&myServer.startTime);

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

    cout << "Starting administration thread here??? \n";
    thread adminThread(adminCommands, &myServer);
    while (ready)
    {
        shared_ptr<cs457::tcpUserSocket> clientSocket;
        int val;
        tie(clientSocket, val) = mysocket.acceptSocket();
        cout << "value for accept is " << val << std::endl;
        cout << "Socket Accepted" << std::endl;
        unique_ptr<thread> t = make_unique<thread>(cclient, clientSocket, id, &myServer);
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
    cout << "Server is shutting down after one client" << endl;
    return 0;
}