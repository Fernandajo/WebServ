/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 16:18:36 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 16:51:30 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/HTTPResponse.hpp"

std::string HTTPResponse::GenerateResponse(const HttpRequest& request)
{
	std::string method = request.GetMethod();
	std::string uri = request.GetRequestURI();
	std::string version = request.GetVersion();

	if (method != "GET")
	{
		SetStatusLine(version, 405, "Method Not Allowed");
		SetBody("<h1>405 Method Not Allowed</h1>");
		SetHeader("Allow", "GET");
		return (ResponseToString());
	}

	// will be replaced with actual file handling logic
	std::string path = "./www" + uri;
	if (path[path.size() - 1] == '/')
		path += "index.html";

	// Check if the file exists
	struct stat fileStat;
	if (stat(path.c_str(), &fileStat) != 0)
	{
		SetStatusLine(version, 404, "Not Found");
		SetBody("<h1>404 Not Found</h1>");
		return (ResponseToString());
	}

	// Open and read file content
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	
}

// Generates the full HTTP response as a string
std::string HTTPResponse::ResponseToString() const
{
	std::ostringstream outputResponse;

	// Start with the status line
	outputResponse << GetStatusLine() << "\r\n";
	
	// Add headers
	std::map<std::string, std::string>::const_iterator it;
	for (it = responseHeaders.begin(); it != responseHeaders.end(); ++it)
		outputResponse << it->first << ": " << it->second << "\r\n";

	// Adds the body if it exists
	outputResponse << "\r\n" << GetBody();

	return (outputResponse.str());
}

// Default constructor
HTTPResponse::HTTPResponse() {}

// Getters
std::string HTTPResponse::GetStatusLine() const { return statusLine; }
std::map<std::string, std::string> HTTPResponse::GetHeaders() const { return responseHeaders; }
std::string HTTPResponse::GetBody() const { return responseBody; }

// Setters

// Sets the status line of the response
void HTTPResponse::SetStatusLine(const std::string& version, int statusCode, const std::string& title) 
{
	std::ostringstream outputLine;
	outputLine << version << " " << statusCode << " " << title;
	this->statusLine = outputLine.str();
}

// Sets a single header in the response
void HTTPResponse::SetHeader(const std::string& key, const std::string& value)
{
	responseHeaders[key] = value;
}

// Sets the body of the response and updates Content-Length header
// temp content-type is text/html, can be changed later
void HTTPResponse::SetBody(const std::string& body)
{
	this->responseBody = body;
	responseHeaders["Content-Length"] = std::to_string(body.size());
	responseHeaders["Content-Type"] = "text/html";
}

