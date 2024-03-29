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
#include <fstream>
#include "tcpUserSocket.h"
#include "tcpServerSocket.h"
#include "Parsing.h"
#include "user.h"
#include "server.h"

using namespace std;

bool ready = true;

bool verbose = true;
bool debug = false;


// handles the client connection. This lives on its own thread and does lots of calling to server command.
int cclient(shared_ptr<cs457::tcpUserSocket> clientSocket, int id, cs457::server *myServer)
{

    bool cont = true; //two means go, 1 means leave, 0 means die.
    
    //Handles the result of the user connection. 
    cs457::user &connectedUser = myServer->addUserWithSocket(clientSocket, &cont);
    if(cont){
        if(verbose){
            cout << "\n[SERVER] Connected user: " << connectedUser.getName() << std::endl;
            cout << "[SERVER] Waiting for message from Client Thread" << id << std::endl;
            cout << "[SERVER]>" <<std::flush;
        }
    }
    else{
        if(verbose){
            cout << "\n[SERVER] Failed password or conflict from: " << connectedUser.getName() << 
            "\n[SERVER]>"<<std::flush;
        }
    }
   
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
            if(verbose)
                cout << "\n[SERVER] Client " << id << " Disconnected\n[SERVER]>"<<std::flush;
            connectedUser.socketActive = false;
            cont = false;
            break;
        }
        //call server command
        //return value tells this thread what to do.
        int ret = myServer->command(msg, connectedUser);
        if (debug)
        {
            cout << "[DEBUG] The client is sending message " << msg << " -- With value return = " << val << endl;
            string s = "[DEBUG] The client is sending message:" + msg + "\n";
            //this sort of sending isn't really necessary. Why thead like this when you join write after, this thread still waits.
            thread childT1(&cs457::tcpUserSocket::sendString, clientSocket.get(), s, true);
            childT1.join();
        }
        else
        {
            cout << "\n[SERVER] waiting for another message \n[SERVER]>" << std::flush;
        }
       
        //deterimine whether we should continue.
        //Has a different returnes. 0 should shutdown, 1 breaks out, 2 continues.
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
        //prints out the users with additional info
        if (message.command == string("USERS"))
        {
            for (auto u : myServer->getUsers())
            {
                //should print out the keys in uMap.
                cout << "Key: " << u.first << endl;
                cout << "Socket: " << u.second.getName() << endl;
                if(u.second.userSocket)
                    cout << "UniqueID: " << u.second.userSocket.get()->getUniqueIdentifier() << endl;
                cout << "Connected: " << u.second.socketActive << endl;
                cout << "Level: " << u.second.getLevel() << endl;
            }
        }
        //sends a ping to the given server
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
        //exist, not gracefully
        else if (message.command == string("EXIT"))
        {
            continueAdmin = false;
            //Kill all threads and disconect clients here!
            exit(0);
        }
        //closes the connection of a given user.
        else if (message.command == "KILL")
        {
            if (myServer->userOnline(message.params[0]))
            {
                cs457::user &victim = myServer->getUser(message.params[0]);
                victim.userSocket.get()->sendString(":SERVER KILL\r\n");
                victim.closeSocket();
                //std::cout << victim.closeSocket();
            }
            else
            {
                std::cout << "Killed user: " << message.params[0];
                std::cout << "\n[SERVER]>" <<std::flush;
            }
        }
        //lists out the users and includes hidden channels and users
        else if (message.command == "CHANNELS")
        {
            std::string channels = myServer->listChannels(/*showusers = */ true);
            cout << "Found channels:\n"
                 << channels;
        }

        //file io described elsewhere.
        else if (message.command == "WUSERS")
        {
            myServer->writeUsers();
        }
        else if (message.command == "RUSERS")
        {
            myServer->readUsers();
        }
        else if (message.command == "WBANS")
        {
            myServer->writeBans();
        }
        else if (message.command == "WCHANNELS")
        {
            myServer->writeChannels();
        }
        else if (message.command == "RCHANNELS")
        {
            myServer->readChannels();
        }
        else if (message.command == "RBANNER")
        {
            myServer->readBanner();
        }
        else if (message.command == "VERBOSE")
        {
            if (message.params.size() > 0)
            {
                if (message.params[0] == "true")
                    verbose = true;
                else 
                    verbose = false;
            }
            else
                verbose = !verbose;
        }
        else
        {
            if(command.length() > 0)
                cout << "[SERVER]> Did not recognize: " << command << endl;
        }
    }
}

int port = 2000;
string configFile;
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

    //parse the config file if we got one.
    if(configFile.length() > 0)
    {
        //we got to parse that bad boy.
        std::ifstream config(configFile);
        
        if(config.is_open())
        {
             std::string line;
             
            while(getline(config, line))
            {
                //ignores comments
                if(line[0] != '#')
                {
                    std::istringstream iss(line);
                    std::string value; std::string attribute;   
                    iss >> attribute;                                        
                    iss >> value;                    
                    if(attribute == "port")
                        port = stoi(value);                    
                    else if (attribute == "dbpath")
                    {
                        db = value;
                        myServer.dbPath = value;
                    }
                }
            }
        }
        else   
            std::cerr << "file could not be opened";
    }

    cout <<"***************Starting Server***************\n";
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

    cout <<"***************Started Server****************\n";
    //lets you see config file success.
    if(debug)
        cout << "VALS " << db << " " << port << endl;

    //spins off the admin thread to allow interactive commands.
    thread adminThread(adminCommands, &myServer);

    //while we continue, accept new connections.
    while (ready)
    {
        shared_ptr<cs457::tcpUserSocket> clientSocket;
        int val;
        tie(clientSocket, val) = mysocket.acceptSocket();
        if(verbose)
        {
            cout << "\n[SERVER] Value for accepted socket is " << val << "\n[SERVER]" <<std::flush;
        }
        unique_ptr<thread> t = make_unique<thread>(cclient, clientSocket, id, &myServer);
        threadList.push_back(std::move(t));

        id++; //not the best way to go about it.
              // threadList.push_back(t);
    }

    for (auto &t : threadList)
    {
        t.get()->join();
    }
    adminThread.join();
    cout << "Server is shutting down after one client" << endl;
    return 0;
}