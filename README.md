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