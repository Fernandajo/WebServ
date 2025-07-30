/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 16:03:47 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/26 20:14:45 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include <fstream>
#include <sstream>
#include <cstdlib>

#include <string>
#include <vector>
#include <map>

#include "Server.hpp"

class ConfigParser
{
	private:
		std::vector<std::string> tokens;
		std::string rawData;
		std::string fileName;
		size_t currentTokenIndex;

		// Helpers to read and tokenize the config file
		void ReadConfigFile();
		void TokenizeConfigFile();

		// Helpers to parse server and routing blocks
		ServerConfig ParseServerBlock();
		RoutingConfig ParseLocationBlock();

		// Helper functions for parsing tokens
		std::string peek();
		std::string next();
		void expect(const std::string& expected);

	public:
		// Constructor
		ConfigParser(const std::string& filename);
		
		// Getter
		std::vector<std::string> GetTokens() const;
		
		// Primary function to parse the config file
		std::vector<ServerConfig> ParseConfigFile();

};


#endif