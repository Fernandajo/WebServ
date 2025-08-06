/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 16:18:36 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/04 00:38:27 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/HTTPResponse.hpp"
#include "../../inc/Server.hpp"

std::string HTTPResponse::GenerateResponse(const HttpRequest& request, Server& server)
{
	std::string method = request.GetMethod();
	std::string uri = request.GetRequestURI();
	std::string version = request.GetVersion();

	const RoutingConfig& route = server.findRouteforURI(uri);

	if (std::find(route.methods.begin(), route.methods.end(), method) == route.methods.end())
	{
		SetErrorResponse(version, 405, "Method Not Allowed", server);
		SetHeader("Allow", "GET, POST, DELETE");
		return (ResponseToString());
	}

	std::string root = route.root.empty() ? server.getRoot() : route.root;
	std::string fullpath = root + uri;

	if (method == "GET")
	{
		std::string path = fullpath;
		struct stat fileStat;
			
		if (stat(path.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
			if (path[path.size() - 1] != '/')
				path += "/";
			path += route.indexFile.empty() ? "index.html" : route.indexFile;
		
			if (stat(path.c_str(), &fileStat) != 0)
			{
				if (route.isAutoIndexOn)
				{
					std::string listing = GenerateDirectoryListing(fullpath, uri);
					SetStatusLine(version, 200, "OK");
					SetBody(listing);
					SetHeader("Content-Type", "text/html");
					return ResponseToString();
				}
				else
				{
					SetErrorResponse(version, 403, "Forbidden", server);
					return ResponseToString();
				}
			}
		} else if (stat(path.c_str(), &fileStat) != 0) {
			SetErrorResponse(version, 404, "Not Found", server);
			return ResponseToString();
		}

		if (!S_ISREG(fileStat.st_mode))
		{
			std::string indexedPath = path;
			if (indexedPath[indexedPath.size() - 1] != '/')
				indexedPath += '/';
			indexedPath += route.indexFile.empty() ? "index.html" : route.indexFile;
			
			struct stat indexedStat;
			if (stat(indexedPath.c_str(), &indexedStat) == 0 && S_ISREG(indexedStat.st_mode))
			{
				path = indexedPath;
			}
			else if (route.isAutoIndexOn)
			{
				std::string directoryListing = GenerateDirectoryListing(fullpath, uri);
				SetStatusLine(version, 200, "OK");
				SetBody(directoryListing);
				SetHeader("Content-Type", "text/html");
				return (ResponseToString());
			}
			else
			{
				SetErrorResponse(version, 403, "Forbidden", server);
				return (ResponseToString());
			}
		}
	
		// Open and read file content
		std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			SetErrorResponse(version, 500, "Internal Server Error", server);
			return (ResponseToString());
		}
	
		std::ostringstream fileContent;
		fileContent << file.rdbuf();
		file.close();
	
		// Sends a success response with the file content
		SetStatusLine(version, 200, "OK");
		SetBody(fileContent.str());
		SetHeader("Content-Type", GetMimeType(path));
		return (ResponseToString());
	}
	else if (method == "POST")
	{
		std::string body = request.GetBody();
	
		if (body.empty()) {
			SetErrorResponse(version, 400, "Bad Request", server);
			return ResponseToString();
		}
	
		// Only handle POSTs to /messages
		if (request.GetPath() != "/messages") {
			SetErrorResponse(version, 404, "Not Found", server);
			return ResponseToString();
		}
	
		std::string decoded = urlDecode(body); // You'll define this below
		std::string message = extractKeyValue(decoded, "message");
	
		if (message.empty()) {
			SetErrorResponse(version, 400, "Missing message", server);
			return ResponseToString();
		}
	
		std::string uploadPath = "./www/messages.txt";
	
		std::ofstream outFile(uploadPath.c_str(), std::ios::app); // append mode
		if (!outFile.is_open()) {
			SetErrorResponse(version, 500, "Internal Server Error", server);
			return ResponseToString();
		}
	
		outFile << message << "\n";
		outFile.close();
	
		// Redirect to messages.html
		SetStatusLine(version, 303, "See Other");
		SetHeader("Location", "/messages.html");
		SetBody("");
		return ResponseToString();
	}
	else if (method == "DELETE")
	{
		// set specific path to delete
		std::string path = fullpath;
		
		// attempt to delete the file
		if (remove(path.c_str()) != 0)
		{
			// If the file cannot be deleted, return an error
			SetErrorResponse(version, 404, "Not Found", server);
			return (ResponseToString());
		}
		
		// If the deletion is successful, return a 204
		SetStatusLine(version, 204, "No Content");
		SetBody("");
		return (ResponseToString());
	}
	else
	{
		// If the method is not supported, return a 405 Not Allowed response
		SetErrorResponse(version, 405, "Method Not Allowed", server);
		return (ResponseToString());
	}
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

// Generates a simple error response
void HTTPResponse::SetErrorResponse(const std::string& version, int code, const std::string& reason, Server& server)
{
	SetStatusLine(version, code, reason);

	std::map<int, std::string>::const_iterator it = server.getErrorPages().find(code);
	if (it != server.getErrorPages().end())
	{
		std::string errorPagePath = server.getRoot() + it->second;
		std::ifstream errorPageFile(errorPagePath.c_str(), std::ios::in);
		if (errorPageFile.is_open())
		{
			std::ostringstream buffer;
			buffer << errorPageFile.rdbuf();
			SetBody(buffer.str());
			errorPageFile.close();
			SetHeader("Content-Type", "text/html");
			return;
		}
	}
	
	std::ostringstream defaultBody;
	defaultBody << "<h1>" << code << " " << reason << "</h1>";
	SetHeader("Content-Type", "text/html");
	SetBody(defaultBody.str());
}

std::string HTTPResponse::GenerateDirectoryListing(const std::string& directoryPath, const std::string& uri)
{
	std::ostringstream responseBody;

	// HTML header
	responseBody << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">";
	responseBody << "<title>Index of " << uri << "</title>";
	responseBody << "<style>"
				 << "body { font-family: monospace; padding: 20px; }"
				 << "table { border-collapse: collapse; width: 100%; }"
				 << "th, td { padding: 8px 12px; border-bottom: 1px solid #ccc; text-align: left; }"
				 << "a { text-decoration: none; color: #0366d6; }"
				 << "</style>";
	responseBody << "</head><body>";
	responseBody << "<h1>Index of " << uri << "</h1>";
	responseBody << "<table><tr><th>Name</th><th>Type</th><th>Size</th></tr>";

	// Open directory
	DIR* dir = opendir(directoryPath.c_str());
	if (!dir)
		return "<h1>500 Internal Server Error</h1>";

	// Optional: Add parent directory link if not root
	if (uri != "/") {
		std::string parent = uri;
		if (parent[parent.size() - 1] == '/')
			parent = parent.substr(0, parent.size() - 1);
		size_t lastSlash = parent.find_last_of('/');
		if (lastSlash != std::string::npos)
			parent = parent.substr(0, lastSlash + 1);
		else
			parent = "/";
		responseBody << "<tr><td><a href=\"" << parent << "\">../</a></td><td>dir</td><td>-</td></tr>";
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string entryName = entry->d_name;
		if (entryName == "." || entryName == "..")
			continue;

		std::string fullPath = directoryPath + "/" + entryName;
		struct stat st;
		if (stat(fullPath.c_str(), &st) == -1)
			continue;

		std::string link = uri;
		if (link[link.size() - 1] != '/')
			link += "/";
		link += entryName;

		std::string displayName = entryName;
		std::string typeStr = "file";
		std::string sizeStr = "-";

		if (S_ISDIR(st.st_mode)) {
			displayName += "/";
			typeStr = "dir";
		} else {
			std::ostringstream sizeBuf;
			sizeBuf << st.st_size;
			sizeStr = sizeBuf.str();
		}

		responseBody << "<tr>"
					 << "<td><a href=\"" << link << "\">" << displayName << "</a></td>"
					 << "<td>" << typeStr << "</td>"
					 << "<td>" << sizeStr << "</td>"
					 << "</tr>";
	}

	closedir(dir);
	responseBody << "</table></body></html>";
	return responseBody.str();
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
	std::ostringstream contentLength;
	contentLength << body.size();
	responseHeaders["Content-Length"] = contentLength.str();
}

