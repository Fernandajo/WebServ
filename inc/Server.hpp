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
		void addClientSocket(int clientSocket);
		//getters
		int getFD();
		int getPort() const;
		std::string getBindHost() const;
		std::string getServerName() const;
		std::string getRoot() const;
		std::map<int, std::string> getErrorPages() const;
		std::vector<RoutingConfig> getRoutes() const;

		//setters
		void setPort(int port);
		void setBindHost(const std::string& bindHost);
		void setServerName(const std::string& serverName);
		void setRoot(const std::string& root);
		void setErrorPage(int errorCode, const std::string& errorPage);
		void setRoute(const RoutingConfig& route);
	
		const RoutingConfig& findRouteforURI(const std::string& uri) const;
};

#endif