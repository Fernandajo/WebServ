#include "../../inc/Server.hpp"

Server::Server() {
	_socketFD = -1;
    _port = PORT; // Default port, can be changed
    _clientSockets.reserve(10); 
}

Server::~Server() {
        if (_socketFD != -1) {
            close(_socketFD);
        }
}

void Server::createSocket()
{
	_socketFD = socket(AF_INET, SOCK_STREAM,0);
    if (_socketFD < 0)  
        throw std::runtime_error("Failed to create socket");
    int opt = 1;
    setsockopt(_socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(_socketFD);
}

void Server::bindEListen() {
    _serverAddr.sin_family = AF_INET;
    // Set the bind address based on host string
    if (_bindHost == "0.0.0.0" || _bindHost == "any") {
        _serverAddr.sin_addr.s_addr = INADDR_ANY;  // Accept from any interface
    } else if (_bindHost == "127.0.0.1" || _bindHost == "localhost") {
        _serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Localhost only
    } else {
        // Try to convert custom IP address
        if (inet_aton(_bindHost.c_str(), &_serverAddr.sin_addr) == 0) {
            std::cerr << "Invalid IP address: " << _bindHost << std::endl;
            close(_socketFD);
            throw std::runtime_error("Failed to convert custom IP address");
        }
    }
    _serverAddr.sin_port = htons(_port);
    if (bind(_socketFD, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0) {
        close(_socketFD);
        throw std::runtime_error("Failed to bind socket");
    }
    if (listen(_socketFD, 5) < 0) {    
        close(_socketFD);
        throw std::runtime_error("Failed to listen on socket");
    }
	std::cout << "Server started on port " << _port << std::endl;
}

void Server::startServer() {
	createSocket();
	bindEListen();
}

void Server::stopServer() {
    close(_socketFD);
    std::cout << "Server stopped." << std::endl;
}

int Server::getFD() { return _socketFD; }
int Server::getPort() { return _port; }
std::string Server::getServerName() { return _serverName; }
std::string Server::getBindHost() { return _bindHost; }

