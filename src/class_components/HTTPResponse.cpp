/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 16:18:36 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/11 21:16:19 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/HTTPResponse.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/helpers.hpp"

static bool endsWith(const std::string& str, const std::string& suffix)
{
	if (suffix.empty() || str.size() < suffix.size())
		return (false);
	return (str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0);
}

static bool isRegularFile(const std::string& path)
{
	struct stat st;
	return (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode));
}

static bool isDirectoryPath(const std::string& path)
{
	struct stat st;
	return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

static void splitUri(const std::string& uri, std::string& path, std::string& query)
{
	std::string::size_type pos = uri.find('?');
	if (pos != std::string::npos)
	{
		path = uri;
		query.clear();
	}
	else
	{
		path = uri.substr(0, pos);;
		query = uri.substr(pos + 1);
	}
}

static std::string chooseIndex(const std::string& dir, const std::vector<std::string>& indexFiles)
{
	for (size_t i = 0; i < indexFiles.size(); ++i)
	{
		std::string path = dir;
		if (!path.empty() && path[path.size() - 1] != '/')
			path += '/';
		path += indexFiles[i];
		if (isRegularFile(path))
			return (path);
	}
	return ("");
}

static bool determineCGI(const std::string& path, const std::string& cgiExt)
{
	return (!cgiExt.empty() && endsWith(path, cgiExt));
}

std::string HTTPResponse::GenerateResponse(const HttpRequest& request, Server& server)
{
	std::string method = request.GetMethod();
	std::string uri = request.GetRequestURI();
	std::string version = request.GetVersion().empty() ? "HTTP/1.1" : request.GetVersion();

	const RoutingConfig& route = server.findRouteforURI(uri);

	if (!route.methods.empty() && std::find(route.methods.begin(), route.methods.end(), method) == route.methods.end())
	{
		SetErrorResponse(version, 405, "Method Not Allowed", server);
		std::string allow = route.methods.empty() ? "GET, POST, DELETE" : std::accumulate(route.methods.begin() + 1, route.methods.end(), route.methods.front(), (std::string(*route.methods.begin()).append(", "), std::plus<std::string>()));
		if (!route.methods.empty())
		{
			allow.clear();
			for (size_t i = 0; i < route.methods.size(); ++i)
			{
				if (i > 0)
					allow.append(", ");
				allow.append(route.methods[i]);
			}
		}
		if (allow.empty())
			allow = "GET, POST, DELETE";
		SetHeader("Allow", allow);
		return (ResponseToString());
	}

	std::string pathPart, queryPart;
	splitUri(uri, pathPart, queryPart);

	std::string root = route.root.empty() ? server.getRoot() : route.root;
	std::string fullpath = root + pathPart;

	if (method == "GET")
	{
		if (isDirectoryPath(fullpath))
		{
			std::string index = chooseIndex(fullpath, route.indexFiles);
			if (!index.empty())
			{
				std::ifstream ifs(index.c_str(), std::ios::in | std::ios::binary);
				if (!ifs.is_open())
				{
					SetErrorResponse(version, 500, "Internal Server Error", server);
					return (ResponseToString());
				}
				std::ostringstream buffer;
				buffer << ifs.rdbuf();
				SetStatusLine(version, 200, "OK");
				SetBody(buffer.str());
				SetHeader("Content-Type", GetMimeType(index));
				return (ResponseToString());
			}
			if (route.isAutoIndexOn)
			{
				std::string listing = GenerateDirectoryListing(fullpath, pathPart);
				SetStatusLine(version, 200, "OK");
				SetBody(listing);
				SetHeader("Content-Type", "text/html");
				return (ResponseToString());
			}
			SetErrorResponse(version, 403, "Forbidden", server);
			return (ResponseToString());
		}

		if (!isRegularFile(fullpath))
		{
			SetErrorResponse(version, 404, "Not Found", server);
			return (ResponseToString());
		}

		if (determineCGI(fullpath, route.cgi_ext))
		{
			int pipefd[2];
			if (pipe(pipefd) == -1)
			{
				SetErrorResponse(version, 500, "Internal Server Error", server);
				return (ResponseToString());
			}

			pid_t pid = fork();
			if (pid == -1)
			{
				close(pipefd[0]);
				close(pipefd[1]);
				SetErrorResponse(version, 500, "Internal Server Error", server);
				return (ResponseToString());
			}
			if (pid == 0)
			{
				//child
				dup2(pipefd[1], STDOUT_FILENO);
				close(pipefd[0]);
				close(pipefd[1]);

				std::string scriptEnv = "SCRIPT_FILENAME=" + fullpath;
				std::string methodEnv = "REQUEST_METHOD=GET";
				std::string protoEnv = "SERVER_PROTOCOL=" + version;
				std::string qsEnv = "QUERY_STRING=" + queryPart;

				char *argv[] = {
					const_cast<char*>(route.cgi_path.c_str()),
					const_cast<char*>(fullpath.c_str()),
					NULL
				};
				char *envp[] = {
					const_cast<char*>(scriptEnv.c_str()),
					const_cast<char*>(methodEnv.c_str()),
					const_cast<char*>(protoEnv.c_str()),
					const_cast<char*>(qsEnv.c_str()),
					NULL
				};
				execve(route.cgi_path.c_str(), argv, envp);
				exit(127);
			}
			
			// parent
			close(pipefd[1]);
			std::string output;
			char buffer[4096];
			ssize_t bytesRead;
			while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
				output.append(buffer, bytesRead);
			close(pipefd[0]);
			waitpid(pid, NULL, 0);

			SetStatusLine(version, 200, "OK");
			return (ResponseFromCGI(output));
		}

		//static
		std::ifstream file(fullpath.c_str(), std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			SetErrorResponse(version, 500, "Internal Server Error", server);
			return (ResponseToString());
		}
		std::ostringstream fileContent;
		fileContent << file.rdbuf();
		file.close();
		
		SetStatusLine(version, 200, "OK");
		SetBody(fileContent.str());
		SetHeader("Content-Type", GetMimeType(fullpath));
		return (ResponseToString());
	}
	else if (method == "POST")
	{
		const std::string& requestBody = request.GetBody();
		
		//CGI POST
		const std::string& cgiLocation = root + pathPart;
		if (determineCGI(cgiLocation, route.cgi_ext))
		{
			int inPipe[2], outPipe[2];
			if (pipe(inPipe) == -1 || pipe(outPipe) == -1)
			{
				if (inPipe[0])
				{
					close(inPipe[0]);
					close(inPipe[1]);
				}
				if (outPipe[0])
				{
					close(outPipe[0]);
					close(outPipe[1]);
				}
				SetErrorResponse(version, 500, "Internal Server Error", server);
				return (ResponseToString());
			}

			pid_t pid = fork();
			if (pid == -1)
			{
				close(inPipe[0]);
				close(inPipe[1]);
				close(outPipe[0]);
				close(outPipe[1]);
				SetErrorResponse(version, 500, "Internal Server Error", server);
				return (ResponseToString());
			}

			std::string contentType = "application/x-www-form-urlencoded";
			std::map<std::string, std::string> headers = request.GetHeaders();
			std::map<std::string, std::string>::const_iterator it = headers.find("Content-Type");
			if (it != headers.end() && !it->second.empty())
				contentType = it->second;
			
			if (pid == 0)
			{
				dup2(inPipe[0], STDIN_FILENO);
				dup2(outPipe[1], STDOUT_FILENO);
				close(inPipe[1]);
				close(outPipe[0]);
				close(inPipe[0]);
				close(outPipe[1]);

				std::ostringstream oss;
				oss << requestBody.size();
				
				std::string scriptEnv = "SCRIPT_FILENAME=" + cgiLocation;
				std::string methodEnv = "REQUEST_METHOD=POST";
				std::string protoEnv = "SERVER_PROTOCOL=" + version;
				std::string contentTypeEnv = "CONTENT_TYPE=" + contentType;
				std::string contentLengthEnv = "CONTENT_LENGTH=" + oss.str();

				char *argv[] = {
					const_cast<char*>(route.cgi_path.c_str()),
					const_cast<char*>(cgiLocation.c_str()),
					NULL
				};

				char *envp[] = {
					const_cast<char *>(scriptEnv.c_str()),
					const_cast<char *>(methodEnv.c_str()),
					const_cast<char *>(protoEnv.c_str()),
					const_cast<char *>(contentTypeEnv.c_str()),
					const_cast<char *>(contentLengthEnv.c_str()),
					NULL
				};
				execve(route.cgi_path.c_str(), argv, envp);
				exit(127);
			}

			//parent
			close(inPipe[0]);
			close(inPipe[1]);

			ssize_t off = 0;
			while (off < (ssize_t)requestBody.size())
			{
				ssize_t bytesWritten = write(inPipe[1], requestBody.data() + off, requestBody.size() - off);
				if (bytesWritten <= 0)
					break;
				off += bytesWritten;
			}
			close(inPipe[1]);

			std::string output;
			char buffer[4096];
			ssize_t bytesRead;
			while ((bytesRead = read(outPipe[0], buffer, sizeof(buffer))) > 0)
				output.append(buffer, bytesRead);
			close(outPipe[0]);
			waitpid(pid, NULL, 0);

			SetStatusLine(version, 200, "OK");
			return (ResponseFromCGI(output));
		}

		// Regular POST handling
		if (route.uploadPath.empty())
		{
			SetErrorResponse(version, 500, "Internal Server Error", server);
			return (ResponseToString());
		}
		std::string destination = route.uploadPath + pathPart;
		std::ofstream out(destination.c_str(), std::ios::out | std::ios::binary);
		if (!out.is_open())
		{
			SetErrorResponse(version, 500, "Internal Server Error", server);
			out.write(requestBody.c_str(), requestBody.size());
			out.close();
		}

		SetStatusLine(version, 204, "No Content");
		SetBody("");
		SetHeader("Connection", "keep-alive");
		return (ResponseToString());
	}
	else if (method == "DELETE")
	{
		// set specific path to delete
		std::string targetPath = root + pathPart;
		// attempt to delete the file
		if (remove(targetPath.c_str()) != 0)
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
	// If the method is not supported, return a 405 Not Allowed response
	SetErrorResponse(version, 405, "Method Not Allowed", server);
	return (ResponseToString());
}
	
// Generates the full HTTP response as a string
std::string HTTPResponse::ResponseToString() const
{
	std::ostringstream outputResponse;

	// Start with the status line
	outputResponse << GetStatusLine() << "\r\n";
	
	// Add headers
	for (std::map<std::string, std::string>::const_iterator it = responseHeaders.begin(); it != responseHeaders.end(); ++it)
		outputResponse << it->first << ": " << it->second << "\r\n";
	// Adds the body if it exists
	outputResponse << "\r\n" << GetBody();

	return (outputResponse.str());
}

std::string HTTPResponse::ResponseFromCGI(const std::string& cgiOutput)
{
	std::string::size_type pos = cgiOutput.find("\r\n\r\n");
	std::string headersPart, bodyPart;
	if (pos == std::string::npos)
	{
		pos = cgiOutput.find("\n\n");
		if (pos == std::string::npos)
		{
			std::ostringstream response;
			response << GetStatusLine() << "\r\n"
					 << "Content-Length: " << cgiOutput.size() << "\r\n"
					 << "\r\n"
					 << cgiOutput;
			return (response.str());
		}
		headersPart = cgiOutput.substr(0, pos);
		bodyPart = cgiOutput.substr(pos);
	}
	else
	{
		headersPart = cgiOutput.substr(0, pos);
		bodyPart = cgiOutput.substr(pos);
	}
	
	std::istringstream headerStream(headersPart);
	std::string line;
	std::ostringstream filteredHeaders;
	bool hasContentLength = false;
	bool hasStatus = false;
	int statusCode = 200;
	std::string reason = "OK";

	while (std::getline(headerStream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;
		std::string::size_type delim = line.find(':');
		if (delim == std::string::npos)
			continue;
		std::string key = line.substr(0, delim);
		std::string value = line.substr(delim + 1);
		while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
			value.erase(0, 1);
		if (key == "Status")
		{
			std::istringstream valueStream(value);
			valueStream >> statusCode;
			std::getline(valueStream, reason);
			if (!reason.empty() && reason[0] == ' ')
				reason.erase(0, 1);
			hasStatus = true;
			continue;
		}
		if (key == "Content-Length")
			hasContentLength = true;
		
		filteredHeaders << key << ": " << value << "\r\n";
	}

	if (hasStatus)
	{
		const std::string& statLine = GetStatusLine();
		std::string::size_type spacePos = statLine.find(' ');
		std::string version = (spacePos != std::string::npos) ? statLine.substr(0, spacePos) : "HTTP/1.1";
		SetStatusLine(version, statusCode, reason);
	}

	if (!hasContentLength)
		filteredHeaders << "Content-Length: " << bodyPart.size() << "\r\n";
	
	std::ostringstream response;
	response << GetStatusLine() << "\r\n"
			 << filteredHeaders.str()
			 << "\r\n"
			 << bodyPart;
	return (response.str());
}

// Generates a simple error response
void HTTPResponse::SetErrorResponse(const std::string& version, int code, const std::string& reason, Server& server)
{
	SetStatusLine(version, code, reason);

	std::map<int, std::string> errorPages = server.getErrorPages();
	std::map<int, std::string>::const_iterator it = errorPages.find(code);
	if (it != errorPages.end())
	{
		std::string errorPagePath = server.getRoot() + it->second;
		std::ifstream errorPageFile(errorPagePath.c_str(), std::ios::in | std::ios::binary);
		if (errorPageFile.is_open())
		{
			std::ostringstream buffer;
			buffer << errorPageFile.rdbuf();
			errorPageFile.close();
			SetBody(buffer.str());
			SetHeader("Content-Type", "text/html");
			return;
		}
	}
	
	std::ostringstream defaultBody;
	defaultBody << "<h1>" << code << " " << (reason.empty() ? "Unknown Error" : reason) << "</h1>";
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

	if (uri != "/")
	{
		std::string parent = uri;
		if (!parent.empty() && parent[parent.size() - 1] == '/')
			parent.erase(parent.size() - 1);
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

		std::string fullPath = directoryPath;
		if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
			fullPath += '/';
		fullPath += entryName;

		struct stat st;
		if (stat(fullPath.c_str(), &st) == -1)
			continue;

		std::string link = uri;
		if (link.empty() && link[link.size() - 1] != '/')
			link += "/";
		link += entryName;

		const bool isDir = S_ISDIR(st.st_mode);
		std::ostringstream sizeBuffer;
		
		if (!isDir)
			sizeBuffer << st.st_size;
		
		responseBody << "<tr>"
					 << "<td><a href=\"" << link << "\">" << entryName << (isDir ? "/" : "") << "</a></td>"
					 << "<td>" << (isDir ? "dir" : "file") << "</td>"
					 << "<td>" << (isDir ? "-" : sizeBuffer.str()) << "</td>"
					 << "</tr>";
	}

	closedir(dir);
	responseBody << "</table></body></html>";
	return (responseBody.str());
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
	outputLine << (version.empty() ? "HTTP/1.1" : version) << " " << statusCode << " " << (title.empty() ? "OK" : title);
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

