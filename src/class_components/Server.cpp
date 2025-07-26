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
    _serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
    _serverAddr.sin_port = htons(_port);
    if (bind(_socketFD, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0) {
        close(_socketFD);
        throw std::runtime_error("Failed to bind socket");
    }
    if (listen(_socketFD, 5) < 0) {    
        close(_socketFD);
        throw std::runtime_error("Failed to listen on socket");
    }

}

void Server::startServer() {
	createSocket();
}
void Server::stopServer() {
    close(_socketFD);
    std::cout << "Server stopped." << std::endl;
}
int Server::getFD() { return _socketFD; }
int Server::getPort() { return _port; }
std::string Server::getServerName() { return _serverName; }
std::string Server::getBindHost() { return _bindHost; }