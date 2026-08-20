/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 16:17:09 by mdomnik           #+#    #+#             */
/*   Updated: 2025/09/24 10:47:39 by nmandakh         ###   ########.fr       */
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
	bool inComment = false;

	//loop through every character in the raw data stream
	for (size_t i = 0; i < rawData.length(); ++i)
	{
		char dataChar = rawData[i];
		
		if (inComment)
		{
			if (dataChar == '\n')
				inComment = false;
			continue;
		}
		if (dataChar == '#')
		{
			if (!buffer.empty())
			{
				tokens.push_back(buffer);
				buffer.clear();
			}
			inComment = true;
			continue;
		}
		// if space is found and buffer is not empty, add buffer to tokens
		if (std::isspace(static_cast<unsigned char>(dataChar)))
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

std::vector<Server> ConfigParser::ParseConfigFile()
{
	std::vector<Server> servers;
	currentTokenIndex = 0;
	while (currentTokenIndex < this->tokens.size())
	{
		if (this->tokens[currentTokenIndex] == "server")
		{
			servers.push_back(ParseServerBlock());
		}
		else
			throw std::runtime_error("Unexpected token: " + this->tokens[currentTokenIndex]);
	}
	return (servers);
}