#include "../../inc/MultiServerManager.hpp"
#include <sys/epoll.h>

MultiServerManager::MultiServerManager() {

}

MultiServerManager::~MultiServerManager() {
   
}

void MultiServerManager::addServers(Server _new) {

}

void MultiServerManager::addServerToEpoll() {
	for (std::vector<std::unique_ptr<Server>>::iterator it = _servers.begin(); it != _servers.end(); it++)
	{
		_ev.events = EPOLLIN;
		_ev.data.fd = _servers.;
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socketFD, &_ev) == -1) {
			close(_socketFD);
			close(_epoll_fd);
			throw std::runtime_error("Failed to add socket to epoll");
		}
	}
	

}

void MultiServerManager::initialize() {
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd == -1) {
		// close servers
        throw std::runtime_error("Failed to create epoll");
    }

    epoll_event events[MAX_CLIENTS];
    sockaddr addr;
    socklen_t addrlen = sizeof(_serverAddr);
    while (true) {
		int numEvents = epoll_wait(_epoll_fd, events, MAX_CLIENTS, -1);
        for (int i = 0; i < numEvents; ++i) {
			if (events[i].data.fd == _socketFD) {
				int clientSocket = accept(_socketFD, &addr, &addrlen);
                if (clientSocket >= 0) {
					_clientSockets.push_back(clientSocket);
                    std::cout << "New client connected." << std::endl;
                }
                set_nonblocking(clientSocket);
                _ev.events = EPOLLIN | EPOLLET;
                _ev.data.fd = clientSocket;
                if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientSocket, &_ev) == -1) {
					std::cerr << "Failed to add client socket to epoll." << std::endl;
                    close(clientSocket);
                }
            }
			else
			{
				std::vector<int>::iterator it;
				for (it = _clientSockets.begin(); it != _clientSockets.end(); ++it)
				{
					if(events[i].data.fd == *it)
						break;
				}
				char buffer[1024];
				int bytesRecv = recv(*it, buffer, sizeof(buffer) - 1, 0);
				if (bytesRecv <= 0){
					closeClientConnection(*it);
					return;
				}
				std::string request(buffer, bytesRecv);
				HttpRequest req;
				ParseStatus status = req.ParseRequestChunk(request);
				if (status == Parse_Success)
				{
					HTTPResponse res;
					std::string response = res.GenerateResponse(req);
					send(*it, response.c_str(), response.size(), 0);
					// maybe close and remove client
				}

            }
		}
	}
    close(_epoll_fd);
    stopServer();
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

