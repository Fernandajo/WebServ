
#include "../inc/MultiServerManager.hpp"

int main(int argc, char* argv[]) {
    if(argc == 2) {
		std::string configFile = argv[1];
        // parsing of config file can be added here
		MultiServerManager serverManager(configFile);
    } else {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }
    return 0;
}


