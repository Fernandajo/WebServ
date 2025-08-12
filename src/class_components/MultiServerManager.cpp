#include "../../inc/MultiServerManager.hpp"
#include <sys/epoll.h>

MultiServerManager::MultiServerManager(std::string& configfile)  {
	ConfigParser parser(configfile);
	_servers = parser.ParseConfigFile();
	std::vector<Server>::iterator it;
	for (it = _servers.begin(); it != _servers.end(); it++)
		it->startServer();
	
	if (_servers.empty()) {
		std::cerr << "No servers found in config file." << std::endl;
		throw std::runtime_error("No servers found in config file.");
	}
	initialize();

}

MultiServerManager::~MultiServerManager() {
   
}


// Adds the server to the epoll instance
// This function is called after the server has been started
// and is ready to accept connections.
// It sets the server socket to non-blocking mode and adds it to the epoll instance
// so that it can listen for incoming connections.
// It also reserves space for client sockets in the server.
void MultiServerManager::addServerToEpoll() {
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
	{
		_ev.events = EPOLLIN | EPOLLET; // Edge-triggered mode
		_ev.data.fd = it->getFD();
		// Set the socket to non-blocking mode
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, it->getFD(), &_ev) == -1) {
			close(it->getFD()); // close the server socket if epoll fails
			close(_epoll_fd);
			throw std::runtime_error("Failed to add socket to epoll");
		}
	}
	

}

// Initializes the epoll and starts listening for events
void MultiServerManager::initialize() {
    _epoll_fd = epoll_create1(0); // creates a instance of epoll I/O multiplex
    if (_epoll_fd == -1) {
		CloseEpoll(); // function to close servers
        throw std::runtime_error("Failed to create epoll");
    }
	// add servers to epoll
	addServerToEpoll();
    epoll_event events[MAX_CLIENTS]; // will handle the events
    sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    while (true) {
		int numEvents = epoll_wait(_epoll_fd, events, MAX_CLIENTS, -1); //waiting for events to happen
        for (int i = 0; i < numEvents; ++i) 
		{
			for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
			{
				if (events[i].data.fd == it->getFD()) 
				{
					int clientSocket = accept(it->getFD(), &addr, &addrlen);
					while (clientSocket > 0 && errno == EAGAIN){
						clientSocket = handleNewClient(clientSocket, it);
						addrlen = sizeof(addr);
						clientSocket = accept(it->getFD(), &addr, &addrlen);
					}
					

				}
            }
			std::map<int, Server*>::iterator it = _client_to_server.find(events[i].data.fd);
			if (it == _client_to_server.end()) {
				std::cerr << "Client not found in server map." << std::endl;
				continue;
			}
			// Handle the client request
			char buffer[1024];
			int bytesRecv = recv(it->first, buffer, sizeof(buffer) - 1, 0);
			if (bytesRecv <= 0){
				closeClientConnection(it->first); // want to pass a ref to function and close and erase.
			}
			std::string request(buffer, bytesRecv);
			HttpRequest req;
			ParseStatus status = req.ParseRequestChunk(request);
			if (status == Parse_Success)
			{
				HTTPResponse res;
				std::string response = res.GenerateResponse(req, *it->second);
				send(it->first, response.c_str(), response.size(), 0);
				while (req.hasMoreData()) {
				    req.StartNextRequest();                // reuse parser, keep leftover buffer
    				ParseStatus ps = req.ParseRequestChunk("");  // continue parsing from internal buffer
    				if (ps == Parse_Success) {
						std::string response = res.GenerateResponse(req, *it->second);
        				send(it->first, response.c_str(), response.size(), 0);
						std::map<std::string, std::string> headers = res.GetHeaders();
						if (headers.find("Connection") != headers.end() && headers["Connection"] == "close") {
							std::cout << "Closing connection for client " << it->first << std::endl;
							closeClientConnection(it->first);
						}
    				} else if (ps == Parse_Incomplete) {
        				break; // need more bytes from socket
    				} else {   // error
        			// generate 4xx/5xx from req.GetErrorMessage(); then break/close
        			break;
    				}
				}
				// maybe close and remove client
			}
		}
	}
	CloseEpoll();
}

// This function is called when a new client connects
// It sets the client socket to non-blocking mode and adds it to the epoll instance
// It also adds the client socket to the server's client sockets vector
int MultiServerManager::handleNewClient(int clientSocket, Server* it) {
	if (clientSocket <= 0)
	{	
		std::cerr << "Failed to accept client connection." << std::endl;
		return -1;
	}
	_client_to_server[clientSocket] = &(*it);
	it->addClientSocket(clientSocket);
	std::cout << "New client connected." << std::endl;
	set_nonblocking(clientSocket);
	_ev.events = EPOLLIN | EPOLLET;
	_ev.data.fd = clientSocket;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientSocket, &_ev) == -1) 
	{
		std::cerr << "Failed to add client socket to epoll." << std::endl;
		close(clientSocket);
	}
}

// Closes the epoll instance and all server sockets
// This function is called when the server is shutting down
// It closes all client sockets and the server socket
// It also closes the epoll instance to free up resources
// It is important to close the epoll instance to avoid memory leaks
// and to ensure that all resources are released properly.
void MultiServerManager::CloseEpoll() {
	std::vector<Server>::iterator it;
	for (it = _servers.begin(); it != _servers.end(); it++)
		it->stopServer();
	// close all server sockets
	if (_epoll_fd != -1) {
		close(_epoll_fd);
		_epoll_fd = -1;
	}
}

void MultiServerManager::closeClientConnection(int clientSocket)
{
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, clientSocket, &_ev);
	// remove from client vec
	close(clientSocket);
	std::cout << "Client " << clientSocket << " disconnected" << std::endl;
}

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


