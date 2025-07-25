/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mime.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 18:06:27 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 18:14:08 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/helpers.hpp"

std::string GetMimeType(const std::string& path)
{
	// Define a map of MIME types
	std::map<std::string, std::string> mimeTypes;

	mimeTypes[".html"] = "text/html";
	mimeTypes[".htm"] = "text/html";
	mimeTypes[".css"] = "text/css";
	mimeTypes[".js"] = "application/javascript";
	mimeTypes[".json"] = "application/json";
	mimeTypes[".png"] = "image/png";
	mimeTypes[".jpg"] = "image/jpeg";
	mimeTypes[".jpeg"] = "image/jpeg";
	mimeTypes[".gif"] = "image/gif";
	mimeTypes[".svg"] = "image/svg+xml";
	mimeTypes[".txt"] = "text/plain";
	mimeTypes[".ico"] = "image/x-icon";
	mimeTypes[".pdf"] = "application/pdf";
	mimeTypes[".zip"] = "application/zip";

	// Find the last occurrence of '.' to get the file extension
	std::string::size_type type = path.rfind('.');

	// if not found, return a default MIME type
	if (type == std::string::npos)
		return ("application/octet-stream");

	// Extract the file extension and check if it exists in the map
	std::string extension = path.substr(type);

	// if it exists in the map, return the corresponding MIME type
	if (mimeTypes.find(extension) != mimeTypes.end())
		return (mimeTypes[extension]);

	// otherwise, return a default MIME type
	return ("application/octet-stream");
}