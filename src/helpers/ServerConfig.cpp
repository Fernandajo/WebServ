/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 20:19:39 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/29 22:45:02 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/Server.hpp"

// Constructor for RoutingConfig
RoutingConfig::RoutingConfig() : isAutoIndexOn(false) {};

// Constructor for ServerConfig
ServerConfig::ServerConfig() : port(8080) {};

// finds the route for a given URI
const RoutingConfig& ServerConfig::findRouteforURI(const std::string& uri) const
{
	size_t highestMatchLength = 0;
	const RoutingConfig* bestMatch = NULL;

	// Iterate through all routes to find the best match
	for (size_t i = 0; i < routes.size(); ++i)
	{
		const RoutingConfig& route = routes[i];
		// Check if the URI starts with the route path
		if (uri.compare(0, route.path.length(), route.path) == 0)
		{
			// If this route is longer than the current best match, update it
			if (route.path.length() > highestMatchLength)
			{
				highestMatchLength = route.path.length();
				bestMatch = &route;
			}
		}
	}
	
	// If no match was found, return the default route or throw an error
	if (!bestMatch)
	{
		for (size_t i = 0; i < routes.size(); ++i)
		{
			if (routes[i].path == "/")
				return (routes[i]);
		}
		throw std::runtime_error("No matching route found for URI: " + uri);
	}
	
	// Return the best matching route
	return (*bestMatch);
}

