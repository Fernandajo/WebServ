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
#include <unordered_map>
#include <memory>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Server.hpp"
#include "helpers.hpp"


#define MAX_CLIENTS 10 // Maximum number of clients 

class MultiServerManager {
private:
	int					_epoll_fd;
	epoll_event			_ev;
	std::vector<std::unique_ptr<Server>> _servers;
    std::unordered_map<int, Server*> fd_to_server; 
	std::map<int, Server*> _client_to_server;
	void addServers(Server _new);
	void addServerToEpoll();
	void initialize();
	void closeClientConnection(int clientSocket);
public:
    // void handleRequest(int clientSocket);
    // void sendResponse(int clientSocket, const std::string& response);
};

void send_response(int client_fd);


#endif // SERVERORG_HPP