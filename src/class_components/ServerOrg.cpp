#include "../../inc/ServerOrg.hpp"
#include <sys/epoll.h>


ServerOrg::ServerOrg() {
    _socketFD = -1;
    _port = 8080; // Default port, can be changed
    _clientSockets.reserve(10); 
}

ServerOrg::~ServerOrg() {
    stopServer();
}

void ServerOrg::stopServer() {

    close(_socketFD);
    std::cout << "Server stopped." << std::endl;
}

void ServerOrg::startServer() {
    _socketFD = socket(AF_INET, SOCK_STREAM,0);
    if (_socketFD < 0)  
        throw std::runtime_error("Failed to create socket");
    int opt = 1;
    setsockopt(_socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(_socketFD);
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
    std::cout << "Server started on port " << _port << std::endl;
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        close(_socketFD);
        throw std::runtime_error("Failed to create epoll");
    }
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = _socketFD;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _socketFD, &ev) == -1) {
        close(_socketFD);
        close(epoll_fd);
        throw std::runtime_error("Failed to add socket to epoll");
    }
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
                    send_response(clientSocket);
                    std::cout << "New client connected." << std::endl;
                }
                set_nonblocking(clientSocket);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientSocket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &ev) == -1) {
                    std::cerr << "Failed to add client socket to epoll." << std::endl;
                    close(clientSocket);
                }
            } else {
                // Handle client requests here
            }
            }
        }
    close(epoll_fd);
    stopServer();
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