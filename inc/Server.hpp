#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <cstring>
#include <netdb.h>
#include <signal.h>
#include <ctime>
#include <cstdlib>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "helpers.hpp"

#define PORT 		8080 // Default port, can be changed

class Server
{
private:
	int					_socketFD;
    struct sockaddr_in	_serverAddr;
    int					_port;
    std::vector<int>	_clientSockets;
	std::string			_serverName;
	std::string			_bindHost;
	void createSocket();
	void bindEListen();
public:
	Server();
	~Server();
    void startServer();
    void stopServer();
	int getFD();
	int getPort();
	std::string getServerName();
	std::string getBindHost();
};


#endif