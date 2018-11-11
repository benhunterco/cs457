all:
	g++-8 -g tcpUserSocket.cpp tcpServerSocket.cpp chatServer.cpp Parsing.cpp user.cpp server.cpp -pthread -o chatServer.out
	g++-8 -g tcpClientSocket.cpp chatClient.cpp client.cpp Parsing.cpp -pthread -o chatClient.out
