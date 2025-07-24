#ifndef CONFIG_HPP
#define  CONFIG_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

typedef struct s_server_config {
	std::string server_name = "default_server";
	std::string host = "localhost";
	std::string root = "./site";
	std::vector<std::string> index;
	std::vector<int> listen;
} t_server_config;

class Config {
private:
	t_server_config server_config;
	std::string config_file_path;
public:
	Config(std::string config_file);
	~Config();

	void printConfig();
};

Config::Config(std::string config_file)
{
	config_file_path = config_file;
	std::ifstream config_file_stream(config_file_path);
	if (!config_file_stream.is_open()) {
		std::cerr << "Error: Could not open config file: " << config_file_path << std::endl;
		return;
	}

	// Reading the configuration file line by line (Should use switch case instead maybe?)
	std::string line;
	while (std::getline(config_file_stream, line)) {
		if (line.find("server_name") != std::string::npos) {
			server_config.server_name = line.substr(line.find(' ') + 1);
		}
		else if (line.find("root") != std::string::npos) {
			server_config.root = line.substr(line.find(' ') + 1);
		}
		else if (line.find("host") != std::string::npos) {
			server_config.host = line.substr(line.find(' ') + 1);
		}
		else if (line.find("index") != std::string::npos) {
			std::istringstream iss(line.substr(line.find(' ') + 1));
			std::string word;
			while (iss >> word) {
				if (word != "index.html")
					server_config.index.push_back(word);
			}
		}
		else if (line.find("listen") != std::string::npos) {
			std::istringstream iss(line.substr(line.find(' ') + 1));

			std::string word;
			while (iss >> word) {
				server_config.listen.push_back(std::stoi(word));
			}
		}
	}
	config_file_stream.close();
}

Config::~Config()
{
}

void	Config::printConfig()
{
	std::cout << "Server Name: " << server_config.server_name << std::endl;
	std::cout << "Host: " << server_config.host << std::endl;
	std::cout << "Root: " << server_config.root << std::endl;
	std::cout << "Index Files: ";
	for (const auto& index : server_config.index) {
		std::cout << index << " ";
	}
	std::cout << std::endl;
	std::cout << "Listen Ports: ";
	for (const auto& port : server_config.listen) {
		std::cout << port << " ";
	}
	std::cout << std::endl;
}

#endif