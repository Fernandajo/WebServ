#ifndef SERVERORG_HPP
#define SERVERORG_HPP

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
#include "ConfigParser.hpp"

#define PORT 		8080 // Default port, can be changed
#define MAX_CLIENTS 10 // Maximum number of clients 

class ServerOrg {
private:
    int					_socketFD;
    struct sockaddr_in	_serverAddr;
    int					_port;
    std::vector<int>	_clientSockets;
	int					epoll_fd;

	epoll_event ev;
	void closeClientConnection(int clientSocket);
	void createSocket();
	void bindEListen();
public:
    ServerOrg();
    ~ServerOrg();
    void startServer();
    void stopServer();  
    // void handleRequest(int clientSocket);
    // void sendResponse(int clientSocket, const std::string& response);
};

void set_nonblocking(int fd);
void send_response(int client_fd);


#endif // SERVERORG_HPP