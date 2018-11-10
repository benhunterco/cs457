# Introduction
The files provided compile to `chatClient.out` and `chatServer.out`. `chatClient.out` provdies a connection to the server that also does some work on its own to make the interaction a little cleaner. These include features that make the client more advanced than just typing IRC commands that are directly sent to the server. `chatServer.out` takes and responds to connections from the clients, as well as providing a prompt for the server user to directly interact with. This readme will cover the behavior, structure, and specific usage of each of these binaries.

# Behavior
This section covers the general behavior of the client and the server and how it may differ from an actual IRC program. 

## Client Behavior
Running the client from the command line with optional arguments presents the user with a prompt of initial commands. The client does not attempt to greate a socket connection right away. This gives the user a chance to adjust their parameters if they want to change those read in from the configuration file or command line. Then when the `/CONNECT` command is issued, the client initiates connection with the server. At this point, IRC styled commands can be sent with `/` as a prefix. Once a `PRIVMSG` has been sent to either a channel or another user, the user can type messages without `/` that will be sent to the last recipient. If the user quits or is disconnected, they are returned to the initial prompt. From here they can change the hostname or password if they wish to attempt reconnect. Or they can quit further. 

## Server Behavior
Running the server from the command line with optional command line arguments presents the user with an interactive prompt. From here, the server administrator can type commands to see the state of the server and read/write to database files. This interaction is meant purely for administration, so there is no need for the `/` prefix to commands, everthing the user types is interpreted as a command. By default the server prints out information to the prompt about connecting users, this can be silenced using `VERBOSE`. 

# Structure
Both the client and the server follow a similar file structure. One source file controls the running of either the client or server and spins off all of the threads. Another controls command sending and recieving. The last few control the sockets. Lastly, Darik's paring code is included to deal with the format of IRC commands(Big thanks!)

## Client Structure
The files used by the client are `chatClient.cpp`, `client.cpp`, `tcpClientSocket.cpp` and `Parsing.cpp` along with the appropriate headers. `chatClient.cpp` contains `main()` and controls the initial interaction with the user. It reads in the command line arguments and the configuration file as well. `client.cpp` handles the sending and receiving of messages, as well as holding stateful information about the user. `tcpClientSocket.cpp` handles the tcp socket layer of sending and recieveing strings. Finally, `Parsing.cpp` is used to parse messages just about everywhere throughout. 

## Server Structure
The files used by the server are `chatServer.cpp`, `server.cpp`, `tcpUserSocket.cpp`, `tcpServerSocket.cpp` and `Parsing.cpp`. `chatServer.cpp` reads in the configuration files and command line arguments to start up the server. It also handles incoming connections to the server and spins up the treads for sending and recieving. Lastly, it provides the interactive prompt and limited interpretation of commands for the administrator. `server.cpp` is a really big file because it handles every command that can be sent by the client, as well as any stateful information about the server. This should probably be broken into different classes, but I did not quite have the time to get there. I recommend using the keys `CTRL-K` + `CTRL-0` in visual studio to collapse all the methods to look at this file. `tcpUserSocket.cpp` and `tcpServerSocket.cpp` provide the tcp connection code that is more complicated for the server. `Parsing.cpp` is also used by the server for the same reasons as the client. 

# Specific Usage
This seciton details how to run each file and provides an explanation of how to run each file from the command line as well as giving a list of commands for both the server and the client. 

## Client. 
The client is run with `./chatClient.out` plus command line arguments. 
* `-u` the most important argument, sets the username. I've set the default to `bobby` for quicker testing, but this argument is needed for users to use a different name.
* `-h` can be used to set the host server address (defaults to 127.0.0.1). 
* `-p` sets the port on which to make the connection (defaults to 2000).
* `-c` sets the config file name. (defaults to chatclient.conf).
* `-t` sets the test file name. This is not yet implemented.
* `-L` sets the log file name. This is not yet implemented.

Once the client is started, an initial prompt is given. From here the user has the following commands. 
* `/CONNECT` Connects the user with the defaults or information from commandline/config file.
* `/CONNECT <hostname> <port>` allows the user to connect to a different host and port if they wish.
* `/CPASS` allows the user to change the password used to connect if they are connecting with a name that is password protected

After the user issues either `/CONNECT` command, they will be connected to the server. Their nickname is automatically added to the servers list of users, unless it is a banned name or password protected. If it is password protected, the server will compare it with the supplied password and accept the connection accordingly. All users supply the default password `@` implicitly, so technically every user is registered with the default password, although they may change it if they wish. Once they are connected, the following list of commands are available. 

* `/AWAY <awaymessage>` sets the optional away message if they would like one.`
* `/DIE` closes the server if the user is a sysop
* `/HELP` returns a short explanation of the basic commands
* `/INFO` returns information about the server, including the banner if the admin has read it in.
* `/INVITE <userName> <#channelName>` lets the user invite another user to a channel. This both sends a message invite to userName, and sets userName as invited if #channelName has the `i` flag set.
* `/ISON <userName>` returns information about whether userName is online.
* `/JOIN <#channelName> <password>` adds the user to channelName, or creates a channel called channelName if one doesn't exist. Here, the '#' is not necessary, although it is everywhere else. If channel name has `p` and `k` set, then password is required (and needs to be correct to join). Otherwise the user does not need to supply one. 
* `/KICK <#channelName> <userName>` kicks userName from #channelName if the user is an operator in the given channel.
* `/KILL <userName>` closes the connection of userName if they are online and the user has sysop permission. 
* `/KNOCK <#channelName>` sends a message to a channel with `i` set that the user would like an invite. (see `/MODE` for more information.
* `/LIST` returns a list of channels that includes topics and their appropriate flags. 
* `/MODE <#channelName||yourUserName> <attribute ... attribute>` is used to set the mode of both users and channels. If the user does not supply a channelName (first parameter does not start with #) it is assumed they are changing their own attributes. Attribute `i` toggles invisible mode, which silences messages and removes them from `/USERS` output, but allows the user to still send commands. Attribute `s` opts the user into or out of recieving NOTICEs. `W` is the same but for `WALLOPS`.
EXAMPLE: /MODE ben i   
    Would toggle my mode to invisible
Otherwise when the first parameter starts with '#' the space spererated list of attributes is interpreted for channels. These attributes don't toggle, instead +/- is used to turn them on our off. `+/-o <userName>` gives or takes operator status to username if the connected user already has operator status, which the creator of a channel has by default. All the rest of these commands also require the requester to be an operator. `+/-p` takes the channel out of or into private mode. This means a password is required to join, which is set with `+k <password>`. If the user sets `p` but not `k` as well , the password is `@`. `+/-s` makes the channel secret, which means it does not show in `/LIST`, but depending on other parameters could still be sent to or joined. `+/-i` makes the channel invite only, invites are sent with `/INVITE`. `+/-t` sets whether everyone can set the topic, or just the operators. `+/-n` is important, it sets whether non members can send to the channel or not. It is useful to use this in tandem with i, or p and k, or s. `+/-b <userName>` bans or unbans userName from the channel. `+/-l <int>` sets the limit to the provided integer. 
EXAMPLE: /MODE #chan +k cooldudesonly +p
would set channel #chan to private with a password of cooldudesonly. 
* `/NICK <newName>` changes the nickname/username of the connected user to newName, given that this is not already taken or password protected. 
* `/NOTICE <userName/#channelName> :message goes here` sends notice to userName or the users in channelName who have `s` set to true. This does not return anything for away users. It does not trigger automatic sending either.
* `/PART <#channelName>` leaves the channel channelName. 
* `/OPER <userName> <operatorPassword>` gives operator priveleges to userName provided operatorPassword is correct. Currently, it is hard coded to `notagoodpassword` on line 548 of `server.cpp`
* `/PASS <password>` sets the password to the user's nickname to password. When reconnecting, they will have to set the client's password variable with `/CPASS`. 
* `/PING` and `/PONG` are covered on the server. 
* `/PRIVMSG <userName/#channelName> :message goes here` sends a message to userName or channelName. After the first message is sent, the user can just type without any command and the message will go to the last recipient. If the wish to switch to another channel or user, they can use `/PRIVMSG <anotherUserName/#anotherChannelName> :message goes here` again. After that, their messages will go directly to anotherUserName or anotherChannelName if no command is given. Recieved PRIVMSGs are printed out directly into the prompt as they come, but their sending channel or user information is differentiated. 
* `/QUIT` disconnects from the server. It leaves the user back at the `/CONNECT` prompt where they can type `/EXIT` to leave entirely. 
* `/RULES` returns the rules for the server. 
* `/SETNAME :Real F. Name` sets the users real name. This is just extra optional information if they want.  
* `/SILENCE <username> <username2> ... <usernameN>` Silences the users in the space seperated list of users. If not usernames are given, it returns the users silence list. 
* `/TIME` returns the time on the server.
* `/TOPIC <#channelName> <:some topic here>` Sets the topic of #channelName to the given topic if allowed. If not topic is given, the topic of channelName is returned. 
* `/USER` is not done in this implementation. The process that user does is done automatically by the connection process.
* `/USERHOST <userName> <userName2> ... <userNameN>` provides information about the users in the given list.
* `/USERIP <userName> <userName2> ... <userNameN>` provides the IP information of all the users in the list. 
* `/uSERS` provides a list of the users on the server.
* `/VERSION` provides version information about the server. 
* `/WALLOPS :message here` sends a message to every sysop or user with `w` set, it behaves similarly to `/NOTICE`.
* `/WHO :real name here` returns all of the users who have the provided real name. 





First thing I'm adding in here.

Joining and sending messages to channels. 
PRIVMSG can be used to send messages to any number of channels and users
    This is what the client will use behind the scenes to establish application like behavior. The user can always to the directly 
    by typing /PRIVMSG users channels message

JOIN is used to join a channel on the server. Essentially, this adds the user to the list of users that the server 
    mainains as its representations of channels. Will set the joined server as active by default. 

CHANNEL will change the active channel for the user. This means if they just type a message without a command it will be sent to the 
    entire channel. Channels can be switched with the /CHANNEL <channelName>

Implementation issue. 
Unless I make really fast progress, I will probably not be able to create buffered recieving of messages on the client.
Instead, the client will show every message as it appears, but with a notification to show the user where it came from.

***************************
MESSAGE FROM user/#channel:
    "Hello there!"
***************************
[client]

## Levels
I implement three out of the four levels directly, and the fourth indirectly. user, sysop and admim are implemented, but
channelop is implemented in a roundabout way. The first person to create a channel is the op by nature of them being the first member. 
This means I don't have to have a special indicator for their status, I just check to see their index in the vector. 
The others are strings in the user class. 

## Modes
operator is treated as a level and not a mode for this implementation, so the 'o' flag is not valid.
Instead, three modes can be set: i, s and w. 
*i sets whether to recieve privmsgs.
*s sets whether to recieve notices.
*w sets whether to recieve wallops.

A user can toggle each mode by sending "/mode [i|s|w]". 

## Silence
usage:
/SILENCE <user> ... <user>
Blocks messages from the users in the list. 

## Part
/PART <channelName> ... <channelName>
Leaves the space sperated list of channels.
