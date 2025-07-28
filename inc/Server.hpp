/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 19:11:38 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/26 20:20:33 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>

// Struct for setting up location blocks; e.g. what path it is and
// what methods can be used in that location
struct RoutingConfig
{
	std::string path;
	std::string root;
	std::string indexFile;
	std::string uploadPath;
	std::vector<std::string> methods;
	bool isAutoIndexOn;

	RoutingConfig();
};

// Struct for seting up a server block
struct ServerConfig
{
	int port;
	std::string root;
	std::map<int, std::string> errorPages;
	std::vector<RoutingConfig> routes;

	ServerConfig();
	
	const RoutingConfig& findRouteforURI(const std::string& uri) const;
};

#endif