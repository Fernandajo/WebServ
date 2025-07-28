// #include "../inc/Webserv.hpp"
// #include "../inc/ServerOrg.hpp"

// int main(int argc, char* argv[]) {
//     if(argc == 2) {
//         // parsing of config file can be added here

//         ServerOrg server;
//         server.startServer();
//     } else {
//         std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
//         return 1;
//     }
//     return 0;
// }

#include "ConfigParser.hpp"
#include "Server.hpp"
#include <iostream>

int main() {
	try {
		ConfigParser parser("config/config.conf");
		std::vector<ServerConfig> servers = parser.ParseConfigFile();

for (size_t i = 0; i < servers.size(); ++i) {
	std::cout << "Server on port: " << servers[i].port << "\n";
	std::cout << "Root: " << servers[i].root << "\n";

	for (size_t j = 0; j < servers[i].routes.size(); ++j) {
		const RoutingConfig& route = servers[i].routes[j];
		std::cout << "  Location: " << route.path << "\n";
		std::cout << "    Methods: ";
		for (size_t k = 0; k < route.methods.size(); ++k)
			std::cout << route.methods[k] << " ";
		std::cout << "\n";
		std::cout << "    Upload path: " << route.uploadPath << "\n";
		std::cout << "    Autoindex: " << (route.isAutoIndexOn ? "on" : "off") << "\n";
	}
}

	} catch (const std::exception& e) {
		std::cerr << "Config error: " << e.what() << std::endl;
	}
	return 0;
}



