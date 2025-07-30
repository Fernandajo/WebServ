/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 16:17:09 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/26 20:15:14 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/ConfigParser.hpp"

// filename constructor
ConfigParser::ConfigParser(const std::string& filename) : fileName(filename), currentTokenIndex(0)
{
	ReadConfigFile();
	TokenizeConfigFile();
}

// Getter for tokens
std::vector<std::string> ConfigParser::GetTokens() const { return tokens; }

// Reads the input from path and returns it
// runtime error is file could not be opened
void ConfigParser::ReadConfigFile()
{
	std::ifstream file(fileName.c_str());
	if (!file.is_open())
		throw (std::runtime_error("Failed to open config file"));
	
	std::ostringstream data;
	data << file.rdbuf();
	file.close();
	rawData = data.str();
}

// Tokenizes the rawData string by delimiting through spaces or characters
void ConfigParser::TokenizeConfigFile()
{
	std::string buffer;

	//loop through every character in the raw data stream
	for (size_t i = 0; i < rawData.length(); ++i)
	{
		char dataChar = rawData[i];
		
		// if space is found and buffer is not empty, add buffer to tokens
		if(isspace(dataChar))
		{
			if (!buffer.empty())
			{
				tokens.push_back(buffer);
				buffer.clear();
			}
		}
		// if any special character is found, add buffer to tokens and
		// tokenize the extra character
		else if (dataChar == '{' || dataChar == '}' || dataChar == ';')
		{
			if (!buffer.empty())
			{
				tokens.push_back(buffer);
				buffer.clear();
			}
			tokens.push_back(std::string(1, dataChar));
		}
		else
			buffer += dataChar;
	}
	if (!buffer.empty())
		tokens.push_back(buffer);
}

std::vector<ServerConfig> ConfigParser::ParseConfigFile()
{
	std::vector<ServerConfig> servers;
	std::vector<std::string> tokens = GetTokens();
	currentTokenIndex = 0;
	while (currentTokenIndex < tokens.size())
	{
		if (tokens[currentTokenIndex] == "server")
		{
			servers.push_back(ParseServerBlock());
		}
		else
			throw std::runtime_error("Unexpected token: " + tokens[currentTokenIndex]);
	}
	return servers;
}