/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser_helper.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmandakh <nmandakh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 19:23:43 by mdomnik           #+#    #+#             */
/*   Updated: 2025/09/18 15:31:47 by nmandakh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../inc/ConfigParser.hpp"

std::string ConfigParser::peek() {
	if (currentTokenIndex >= tokens.size())
	{
		std::ostringstream oss;
		oss << "Unexpected EOF at token index " << currentTokenIndex;
		throw std::runtime_error(oss.str());
	}
	return tokens[currentTokenIndex];
}

std::string ConfigParser::next() {
	if (currentTokenIndex >= tokens.size())
	{
		std::ostringstream oss;
		oss << "Unexpected EOF at token index " << currentTokenIndex;
		throw std::runtime_error(oss.str());
	}
	return tokens[currentTokenIndex++];
}

void ConfigParser::expect(const std::string& expected)
{
	if (currentTokenIndex >= tokens.size())
	{
		std::ostringstream oss;
		oss << "Unexpected EOF at token index " << currentTokenIndex;
		throw std::runtime_error(oss.str());
	}
	const std::string& got = tokens[currentTokenIndex];
	if (got != expected)
		throw std::runtime_error("Unexpected token: expected '" + expected + "', got '" + got + "'");
	++currentTokenIndex;
}

Server ConfigParser::ParseServerBlock()
{
	Server serverConfig;
	expect("server");
	expect("{");
	while (peek() != "}")
	{
		std::string token = next();
		
		if (token == "listen")
		{
			const std::string temp = next();
			char *endPoint = 0;
			long port = std::strtol(temp.c_str(), &endPoint, 10);
			if (!endPoint || *endPoint != '\0' || port < 1 || port > 65535)
			{
				throw std::runtime_error("Invalid port number: " + temp);
			}
			serverConfig.setPort(static_cast<int>(port));
			expect(";");
		}
		else if (token == "root")
		{
			serverConfig.setRoot(next());
			expect(";");
		}
		else if (token == "host")
		{
			serverConfig.setBindHost(next());
			expect(";");
		}
		else if (token == "server_name")
		{
			std::vector<std::string> names;
			while (peek() != ";")
			{
				names.push_back(next());
			}
			expect(";");
			if (names.empty())
			{
				throw std::runtime_error("server_name must not be empty");
			}
			serverConfig.setServerNames(names);
		}
		else if (token == "error_page")
		{
			const std::string errorCodeStr = next();
			char *endPoint = 0;
			long code = std::strtol(errorCodeStr.c_str(), &endPoint, 10);
			if (!endPoint || *endPoint != '\0' || code < 100 || code > 599)
			{
				throw std::runtime_error("Invalid error code: " + errorCodeStr);
			}
			std::string errorPage = next();
			serverConfig.setErrorPage(static_cast<int>(code), errorPage);
			expect(";");
		}
		else if (token == "location")
		{
			serverConfig.addRoute(ParseLocationBlock());
		}
		else
		{
			throw std::runtime_error("Unexpected token in server block: " + token);
		}
	}

	expect("}");
	return (serverConfig);
}

RoutingConfig ConfigParser::ParseLocationBlock()
{
	RoutingConfig routingConfig;

	routingConfig.path = next();
	if (routingConfig.path.empty() || routingConfig.path[0] != '/')
		throw std::runtime_error("location path must start with '/': " + routingConfig.path);
	expect("{");
	
	while (peek() != "}")
	{
		std::string token = next();
		
		if (token == "allow_methods")
		{
			static const char* methodsArr[] = {"GET", "POST", "DELETE"};
			static const std::set<std::string> validMethods(methodsArr, methodsArr + 3);
			while (peek() != ";")
			{
				std::string method = next();
				if (!validMethods.count(method))
				{
					throw std::runtime_error("Invalid method in location block: " + method);
				}
				routingConfig.methods.push_back(method);
			}
			expect(";");
		}
		else if (token == "upload_path")
		{
			routingConfig.uploadPath = next();
			expect(";");
		}
		else if (token == "root")
		{
			routingConfig.root = next();
			expect(";");
		}
		else if (token == "autoindex")
		{
			std::string state = next();
			if (state == "on")
				routingConfig.isAutoIndexOn = true;
			else if (state == "off")
				routingConfig.isAutoIndexOn = false;
			else
				throw std::runtime_error("Invalid autoindex state: " + state);
			expect(";");
		}
		else if (token == "index")
		{
			std::vector<std::string> files;
			while (peek() != ";")
			{
				files.push_back(next());
			}
			expect(";");
			routingConfig.indexFiles = files;
		}
		else if (token == "cgi_path")
		{
			routingConfig.cgi_path = next();
			expect(";");
		}
		else if (token == "cgi_extension")
		{
			routingConfig.cgi_ext = next();
			if (routingConfig.cgi_ext.empty() || routingConfig.cgi_ext[0] != '.')
				throw std::runtime_error("CGI extension must start with '.': " + routingConfig.cgi_ext);
			expect(";");
		}
		else
		{
			throw std::runtime_error("Unexpected token in location block: " + token);
		}
	}
	expect("}");
	return (routingConfig);
}