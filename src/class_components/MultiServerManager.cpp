#include "../../inc/MultiServerManager.hpp"
#include <sys/epoll.h>

MultiServerManager::MultiServerManager() {

}

MultiServerManager::~MultiServerManager() {
   
}

void MultiServerManager::startServer() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
		close(_socketFD);
        throw std::runtime_error("Failed to create epoll");
    }
    ev.events = EPOLLIN;
    ev.data.fd = _socketFD;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _socketFD, &ev) == -1) {
		close(_socketFD);
        close(epoll_fd);
        throw std::runtime_error("Failed to add socket to epoll");
    }
	std::cout << "Server started on port " << _port << std::endl;
    epoll_event events[MAX_CLIENTS];
    sockaddr addr;
    socklen_t addrlen = sizeof(_serverAddr);
    while (true) {
		int numEvents = epoll_wait(epoll_fd, events, MAX_CLIENTS, -1);
        for (int i = 0; i < numEvents; ++i) {
			if (events[i].data.fd == _socketFD) {
				int clientSocket = accept(_socketFD, &addr, &addrlen);
                if (clientSocket >= 0) {
					_clientSockets.push_back(clientSocket);
                    std::cout << "New client connected." << std::endl;
                }
                set_nonblocking(clientSocket);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientSocket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &ev) == -1) {
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
    close(epoll_fd);
    stopServer();
}

void MultiServerManager::closeClientConnection(int clientSocket)
{
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, clientSocket, &ev);
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

