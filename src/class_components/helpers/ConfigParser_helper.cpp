/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser_helper.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moojig12 <moojig12@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 19:23:43 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/03 02:14:51 by moojig12         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../inc/ConfigParser.hpp"

std::string ConfigParser::peek() {
	if (currentTokenIndex >= tokens.size())
		throw std::runtime_error("No more tokens available");
	return tokens[currentTokenIndex];
}

std::string ConfigParser::next() {
	if (currentTokenIndex >= tokens.size())
		throw std::runtime_error("No more tokens available");
	return tokens[currentTokenIndex++];
}

void ConfigParser::expect(const std::string& expected) {
	if (next() != expected)
		throw std::runtime_error("Unexpected token: expected '" + expected + "', got '" + peek() + "'");
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
			serverConfig.setPort(std::atoi(next().c_str()));
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
			serverConfig.setServerName(next());
			expect(";");
		}
		else if (token == "error_page")
		{
			int errorCode = std::atoi(next().c_str());
			std::string errorPage = next();
			serverConfig.setErrorPage(errorCode, errorPage);
			expect(";");
		}
		else if (token == "location")
		{
			serverConfig.setRoute(ParseLocationBlock());
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
	expect("{");
	
	while (peek() != "}")
	{
		std::string token = next();
		
		if (token == "allow_methods")
		{
			while (peek() != ";")
			{
				routingConfig.methods.push_back(next());
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
			expect(";");
		}
		else if (token == "index")
		{
			routingConfig.indexFile = next();
			expect(";");
		}
		else if (token == "cgi_path")
		{
			routingConfig.cgi_path = next();
			expect(";");
		}
		else if (token == "cgi_extension")
		{
			routingConfig.cgi_ext = next();
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