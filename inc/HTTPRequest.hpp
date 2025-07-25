/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/24 23:58:52 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 05:02:35 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <string>
#include <cstdlib>
#include <algorithm>
#include <map>

#define RequestEOL "\r\n"
#define HeaderEOL  "\r\n\r\n"

// Return values for HTTP parsing
enum ParseStatus
{
	Parse_Success,
	Parse_Incomplete,
	Parse_BadRequest,
	Parse_NotImplemented,
	Parse_VersionNotSupported
};

// State Tracking
enum ParseState
{
	PARSE_REQUEST_LINE,
	PARSE_HEADERS,
	PARSE_BODY,
	PARSE_DONE,
	PARSE_ERROR
};

// Class for storing parsed information from HTTP Requests
class HttpRequest
{
	// Split components of the raw input in valid format
	private:
		std::string method;
		std::string requestURI;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;

		std::string errorMessage;
		ParseState state;
		std::string buffer;

	public:
	
		//getters
		std::string GetMethod() const;
		std::string GetRequestURI() const;
		std::string GetVersion() const;
		std::map<std::string, std::string> GetHeaders() const;
		std::string GetBody() const;

		std::string GetErrorMessage() const;
		ParseState getState() const;

		// setters
		void SetMethod(const std::string& method);
		void SetRequestURI(const std::string& requestURI);
		void SetVersion(const std::string& version);
		void SetHeaders(const std::map<std::string, std::string>& headers);
		void SetBody(const std::string& body);

		void SetErrorMessage(const std::string& errorMessage);
		
	
		//default constructor
		HttpRequest();

		//member function
		bool isComplete() const;
		bool hasError() const;
		ParseStatus ValidateContentLength();
		
		ParseStatus ParseRequestLine(const std::string& requestLine);
		ParseStatus ParseHeaders(const std::string& headers);
		ParseStatus ParseBody(const std::string& body);

		ParseStatus ParseRequestChunk(const std::string& chunk);

		std::string generateSimpleResponse() const;
};

#endif