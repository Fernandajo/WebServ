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
#include <string>
#include <map>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "helpers.hpp"

#define PORT 		8080 // Default port, can be changed

// Struct for setting up location blocks; e.g. what path it is and
// what methods can be used in that location
struct RoutingConfig
{
	std::string path;
	std::string root;
	std::string indexFile;
	std::string uploadPath;
	std::string cgi_path;
	std::string cgi_ext;
	std::vector<std::string> methods;
	bool isAutoIndexOn;

	RoutingConfig();
};

// Struct for seting up a server block
struct ServerConfig
{
	int port;
	std::string host;
	std::string serverName;
	std::string root;
	std::map<int, std::string> errorPages;
	std::vector<RoutingConfig> routes;

	ServerConfig();
	
	const RoutingConfig& findRouteforURI(const std::string& uri) const;
};

class Server
{
private:
	int							_socketFD;
	struct sockaddr_in			_serverAddr;
	std::vector<int>			_clientSockets;

	int							_port;
	std::string					_bindHost;
	std::string					_serverName;
	std::string					_root;
	std::map<int, std::string>	_errorPages;
	std::vector<RoutingConfig>	_routes;
	
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
	const RoutingConfig& findRouteforURI(const std::string& uri) const;
};

#endif