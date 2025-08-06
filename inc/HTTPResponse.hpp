/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moojig12 <moojig12@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 16:13:22 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/04 17:13:25 by moojig12         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fstream>
#include "HTTPRequest.hpp"
#include "Server.hpp"
#include "helpers.hpp"

// class for generating HTTP responses
class HTTPResponse
{
	private:
		std::string statusLine;
		std::map<std::string, std::string> responseHeaders;
		std::string responseBody;
	public:
		// Default constructor
		HTTPResponse();
		
		// Getters
		std::string GetStatusLine() const;
		std::map<std::string, std::string> GetHeaders() const;
		std::string GetBody() const;

		// Setters
		void SetStatusLine(const std::string& version, int statusCode, const std::string& title);
		void SetHeader(const std::string& key, const std::string& value);
		void SetBody(const std::string& body);

		// Generate the full HTTP response as a string
		std::string GenerateResponse(const HttpRequest& request, Server& serverConfig);

		// packages the response into a single string
		std::string ResponseToString() const;
		std::string	ResponseFromCGI(const std::string& cgiOutput);
		// Generates a simple error response
		void SetErrorResponse(const std::string& version, int code, const std::string& reason, Server& server);

		// Generate Directory Listing Response
		std::string GenerateDirectoryListing(const std::string& directoryPath, const std::string& uri);
};

#endif