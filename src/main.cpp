#include "../inc/Webserv.hpp"
#include "../inc/ServerOrg.hpp"

int main(int argc, char* argv[]) {
    if(argc == 2) {
        // parsing of config file can be added here

        ServerOrg server;
        server.startServer();
    } else {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }
    return 0;
}
