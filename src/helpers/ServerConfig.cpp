/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 20:19:39 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/26 20:25:02 by mdomnik          ###   ########.fr       */
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

	for (size_t i = 0; i < routes.size(); ++i)
	{
		const RoutingConfig& route = routes[i];
		if (uri.compare(0, route.path.length(), route.path) == 0)
		{
			if (route.path.length() > highestMatchLength)
			{
				
			}
		}
	}
}

