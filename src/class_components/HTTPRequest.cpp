/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 00:08:55 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 03:35:03 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/HTTPRequest.hpp"
#include "../../inc/helpers/HTTPRequestHelper.hpp"

#include <iostream>


// Parse request function, which takes the raw request and populates
// private variables with necessary information.
// return adequate parsing status
ParseStatus HttpRequest::ParseRequest(const std::string& raw)
{
	size_t eol = raw.find_first_of(RequestEOL);
	if (eol == std::string::npos)
	{
		errorMessage = "Malformed request: no end of line found";
		return (Parse_BadRequest);
	}
	//finds the request line, which is the first line of the request
	std::string requestLine = raw.substr(0, eol);
	
	// attempts to parse the request line
	ParseStatus status = ParseRequestLine(requestLine);
	if (status != Parse_Success)
		return (status);

	// finds the beggining of headers
	size_t headersStart = raw.find_first_of(RequestEOL) + 2;
	size_t headersEnd = raw.find(HeaderEOL);
	
	// checks if headers are present
	if (headersEnd == std::string::npos)
	{
		errorMessage = "Malformed request: headers not terminated correctly";
		return (Parse_BadRequest);
	}
	
	// extracts the headers from the raw request
	std::string headersRaw = raw.substr(headersStart, headersEnd - headersStart);
	
	// attempts to parse the headers
	status = ParseHeaders(headersRaw);
	if (status != Parse_Success)
		return (status);

	// extracts the body if present and the method is POST
	if (GetMethod() == "POST" || GetMethod() == "PUT")
	{
		size_t bodyStart = headersEnd + 4; // 4 is the length of HeaderEOL
		std::string bodyRaw = raw.substr(bodyStart);
		status = ParseBody(bodyRaw);
		if (status != Parse_Success)
			return (status);
	}
	return (Parse_Success);
}

// Parses the request line into method, URI, and version
// returns adequate parsing status
ParseStatus HttpRequest::ParseRequestLine(const std::string& requestLine)
{
	// Check if the request line is empty
	if (requestLine.empty())
	{
		errorMessage = "Malformed request: empty request line";
		return (Parse_BadRequest);
	}

	// Split the request line into components
	std::vector<std::string> requestComponents = HTTPRequestHelper::splitRequestLine(requestLine);
	
	// Check if we have exactly 3 components: method, URI, and version
	if (requestComponents.size() != 3)
	{
		errorMessage = "Malformed request line: " + requestLine;
		return (Parse_BadRequest);
	}

	//validate method, URI, and version
	if (!HTTPRequestHelper::isValidMethod(requestComponents[0]))
	{
		errorMessage = "Unsupported HTTP method: " + requestComponents[0];
		return (Parse_NotImplemented);
	}
	SetMethod(requestComponents[0]);

	if (!HTTPRequestHelper::isValidRequestURI(requestComponents[1]))
	{
		errorMessage = "Invalid request URI: " + requestComponents[1];
		return (Parse_BadRequest);
	}
	SetRequestURI(requestComponents[1]);

	if (!HTTPRequestHelper::isValidVersion(requestComponents[2]))
	{
		errorMessage = "Unsupported HTTP version: " + requestComponents[2];
		return (Parse_VersionNotSupported);
	}
	SetVersion(requestComponents[2]);
	
	// If all checks passed, return success
	return (Parse_Success);
}

// Parses the headers from the raw request
// rejects malformed headers and duplicates
// returns adequate parsing status
ParseStatus HttpRequest::ParseHeaders(const std::string& headersRaw)
{
	int headerIndex = 0;
	std::string headerLine = HTTPRequestHelper::getHeaderLine(headersRaw, headerIndex);

	std::map<std::string, std::string> headersParsed;
	if (headerLine.empty())
	{
		errorMessage = "No headers found in the request";
		return (Parse_BadRequest);
	}
	while (!headerLine.empty())
	{
		// find the delimiter for key-value pairs
		size_t colonPos = headerLine.find(':');
		if (colonPos == std::string::npos)
		{
			errorMessage = "Malformed header line: " + headerLine;
			return (Parse_BadRequest);
		}
		
		// Extract key and value from the header line and trim them
		std::string key = headerLine.substr(0, colonPos);
		std::string value = headerLine.substr(colonPos + 1);
		
		key = HTTPRequestHelper::trim(key);
		value = HTTPRequestHelper::trim(value);
		
		// Check if key or value is empty
		if (key.empty() || value.empty())
		{
			errorMessage = "Empty key or value in header: " + headerLine;
			return (Parse_BadRequest);
		}

		//changes all keys to lower-case for compatibility
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		// Check for duplicate headers
		if (headersParsed.find(key) != headersParsed.end())
		{
			errorMessage = "Duplicate header field: " + key;
			return (Parse_BadRequest);
		}

		// Store the header in the map
		headersParsed[key] = value;
		headerLine = HTTPRequestHelper::getHeaderLine(headersRaw, ++headerIndex);
	}
	SetHeaders(headersParsed);
	return (Parse_Success);
}

// Parses the request body based on Content-Length header
// Assumes bodyRaw starts exactly at the body (after headers)
// Returns Parse_BadRequest on invalid or short body
ParseStatus HttpRequest::ParseBody(const std::string& body)
{
	//checks if the Content-Length header is present as it is required by the body
	std::map<std::string, std::string> headers = GetHeaders();
	std::map<std::string, std::string>::const_iterator it = headers.find("content-length");

	if (it == headers.end())
	{
		errorMessage = "Missing Content-Length for request with body";
		return (Parse_BadRequest);
	}
	
	//checks if the content size is not 0 or negative
	int contentLength = std::atoi(it->second.c_str());
	if (contentLength < 1)
	{
		errorMessage = "Invalid Content-Length: " + it->second;
		return (Parse_BadRequest);
	}

	// checks if content length does not exceed 1mb
	const int maxContentLength = 1 * 1024 * 1024;
	if (contentLength > maxContentLength)
	{
		errorMessage = "Payload too large: " + it->second + " bytes exceeds 1MB limit";
		return (Parse_BadRequest);
	}
	
	// checks if the content length does not exceed the body
	if ((int)body.length() < contentLength)
	{
		errorMessage = "Body too short: expected " + it->second + " bytes";
		return (Parse_BadRequest);
	}

	SetBody(body.substr(0, contentLength));
	return (Parse_Success);
}

// Default constructor
HttpRequest::HttpRequest() {}

// Getters
std::string HttpRequest::GetMethod() const { return method; }

std::string HttpRequest::GetRequestURI() const { return requestURI; }

std::string HttpRequest::GetVersion() const { return version; }

std::map<std::string, std::string> HttpRequest::GetHeaders() const { return headers; }

std::string HttpRequest::GetBody() const { return body; }

std::string HttpRequest::GetErrorMessage() const { return errorMessage; }

// Setters
void HttpRequest::SetMethod(const std::string& method) { this->method = method; }

void HttpRequest::SetRequestURI(const std::string& requestURI) { this->requestURI = requestURI; }

void HttpRequest::SetVersion(const std::string& version) { this->version = version; }

void HttpRequest::SetHeaders(const std::map<std::string, std::string>& headers) { this->headers = headers; }

void HttpRequest::SetBody(const std::string& body) { this->body = body; }

void HttpRequest::SetErrorMessage(const std::string& errorMessage) { this->errorMessage = errorMessage; }