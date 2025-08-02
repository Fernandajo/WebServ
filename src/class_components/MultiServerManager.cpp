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
		_ev.events = EPOLLIN;
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
		//  add function to close servers
        throw std::runtime_error("Failed to create epoll");
    }
	// add servers to epoll
	addServerToEpoll();
    epoll_event events[MAX_CLIENTS]; // will handle the events
    sockaddr addr;
    socklen_t addrlen = sizeof(addr);
	bool serverAdded = false;
    while (true) {
		int numEvents = epoll_wait(_epoll_fd, events, MAX_CLIENTS, -1); //waiting for events to happen
        for (int i = 0; i < numEvents; ++i) 
		{
			serverAdded = false;
			for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
			{
				if (events[i].data.fd == it->getFD()) 
				{
					serverAdded = true;
					int clientSocket = accept(it->getFD(), &addr, &addrlen);
                	if (clientSocket >= 0)
					{
						_client_to_server[clientSocket] = &(*it);
						it->addClientSocket(clientSocket);
                    	std::cout << "New client connected." << std::endl;
					}
					set_nonblocking(clientSocket);
					_ev.events = EPOLLIN | EPOLLET;
					_ev.data.fd = clientSocket;
					if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientSocket, &_ev) == -1) 
					{
						std::cerr << "Failed to add client socket to epoll." << std::endl;
						close(clientSocket);
					}
				}
            }
			if (serverAdded == true) {
				std::cerr << "Server found for the event." << std::endl;
				continue;
			}
			// std::vector<int>::iterator it;
			// for (it = _clientSockets.begin(); it != _clientSockets.end(); ++it)
			// 	{
			// 		if(events[i].data.fd == *it)
			// 			break;
			// 	}
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
				return;
			}
			std::string request(buffer, bytesRecv);
			HttpRequest req;
			ParseStatus status = req.ParseRequestChunk(request);
			if (status == Parse_Success)
			{
				HTTPResponse res;
				std::string response = res.GenerateResponse(req, *it->second);
				send(it->first, response.c_str(), response.size(), 0);
				std::map<std::string, std::string> headers = res.GetHeaders();
				if (headers.find("Connection") != headers.end() && headers["Connection"] == "close") {
					std::cout << "Closing connection for client " << it->first << std::endl;
					closeClientConnection(it->first);
				}
				// maybe close and remove client
			}
		}
	}
    close(_epoll_fd);
    // stopServer();
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

void send_response(int client_fd) {
    std::string body = "<html><body><h1>Hello, Friends!</h1></body></html>";
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 48 \r\n"
        "Connection: keep-alive\r\n"
        "\r\n" +
        body;

    send(client_fd, response.c_str(), response.size(), 0);
}

