/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 00:08:55 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 16:13:44 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/HTTPRequest.hpp"
#include "../../inc/helpers/HTTPRequestHelper.hpp"

// Parses the raw HTTP request in one go
ParseStatus HttpRequest::ParseRawRequest(const std::string& raw)
{
	ParseStatus status;
	status = ParseRequestChunk(raw);
	if (status != Parse_Success)
		return (status);
	if (!isComplete()) {
		errorMessage = "Incomplete HTTP request.";
		state = PARSE_ERROR;
		return (Parse_BadRequest);
	}
	return (Parse_Success);
}

// Parses incoming chunks of an HTTP request progressively by chunks tracking the current state
// Accumulates data in buffer and parses once enough is available
ParseStatus HttpRequest::ParseRequestChunk(const std::string& chunk)
{
	// Append the new chunk to the buffer
	buffer += chunk;

	if (buffer.empty())
	{
		errorMessage = "Empty request chunk received";
		return (Parse_BadRequest);
	}
	
	// If we are in the request line state, we need to parse the request line first
	if (state == PARSE_REQUEST_LINE) {
		size_t eol = buffer.find("\r\n");
		if (eol == std::string::npos)
		{
			errorMessage = "Request line is incomplete";
			return (Parse_Incomplete);
		}

		// Extract the request line from the buffer
		std::string requestLine = buffer.substr(0, eol);
		ParseStatus status = ParseRequestLine(requestLine);
		if (status != Parse_Success) {
			state = PARSE_ERROR;
			return (status);
		}

		// Remove the request line from the buffer
		buffer.erase(0, eol + 2);
		state = PARSE_HEADERS;
	}

	// If we are in the headers state, we need to parse the headers
	if (state == PARSE_HEADERS) {
		size_t headerEnd = buffer.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
		{
			errorMessage = "Headers are incomplete";
			return (Parse_Incomplete);
		}

		// Extract the headers from the buffer
		std::string headersRaw = buffer.substr(0, headerEnd);
		ParseStatus status = ParseHeaders(headersRaw);
		if (status != Parse_Success) {
			state = PARSE_ERROR;
			return (status);
		}
		// Remove the headers from the buffer
		buffer.erase(0, headerEnd + 4);

		// If there is no body, we are done
		if (method == "POST") {
			state = PARSE_BODY;
		} else {
			state = PARSE_DONE;
			return (Parse_Success);
		}
	}

	// If we are in the body state, we need to parse the body
	if (state == PARSE_BODY) {
		// Validate Content-Length value even if body isn't fully received
		ParseStatus status = ValidateContentLength();
		if (status != Parse_Success) {
			state = PARSE_ERROR;
			return (status);
		}

		// If the buffer is empty, we are incomplete
		int contentLength = std::atoi(headers["content-length"].c_str());
		if ((int)buffer.size() < contentLength)
		{
			errorMessage = "Body is incomplete";
			return (Parse_Incomplete);
		}

		// Body is fully available, now extract it
		status = ParseBody(buffer.substr(0, contentLength));
		if (status != Parse_Success) {
			state = PARSE_ERROR;
			return (status);
		}

		state = PARSE_DONE;
		return (Parse_Success);
	}

	if (state == PARSE_DONE)
		return (Parse_Success);
	if (state == PARSE_ERROR)
		return (Parse_BadRequest);

	return (Parse_Incomplete);
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
	
	int contentLength = std::atoi(it->second.c_str());

	// checks if content length does not exceed 1mb
	const int maxContentLength = 1 * 1024 * 1024;
	if (contentLength > maxContentLength)
	{
		errorMessage = "Payload too large: " + it->second + " bytes exceeds 1MB limit";
		return (Parse_BadRequest);
	}
	
	//checks if the content size is not 0 or negative
	if (contentLength < 1)
	{
		errorMessage = "Invalid Content-Length: " + it->second;
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

// Validates the Content-Length header and checks if it exceeds the limit
ParseStatus HttpRequest::ValidateContentLength()
{
	std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
	// If Content-Length header is missing, return error
	if (it == headers.end())
	{
		errorMessage = "Missing Content-Length for body";
		return (Parse_BadRequest);
	}

	int contentLength = std::atoi(it->second.c_str());
	// If Content-Length is not a valid number or is less than 1, return error
	if (contentLength <= 0)
	{
		errorMessage = "Invalid Content-Length: " + it->second;
		return (Parse_BadRequest);
	}

	const int maxContentLength = 1 * 1024 * 1024;
	// If Content-Length exceeds the maximum allowed size, return error
	if (contentLength > maxContentLength)
	{
		errorMessage = "Payload too large: " + it->second + " bytes exceeds 1MB limit";
		return (Parse_BadRequest);
	}

	return (Parse_Success);
}


// Generates a simple HTTP response with the given body
std::string HttpRequest::generateSimpleResponse() const {
	if (method == "GET" && !requestURI.empty()) {
		return "HTTP/1.1 200 OK\r\n"
		       "Content-Type: text/plain\r\n"
		       "Content-Length: 13\r\n"
		       "\r\n"
		       "Hello, world";
	}
	return "";
}


// Default constructor
HttpRequest::HttpRequest() : state(PARSE_REQUEST_LINE) {}

// Getters
std::string HttpRequest::GetMethod() const { return method; }

std::string HttpRequest::GetRequestURI() const { return requestURI; }

std::string HttpRequest::GetVersion() const { return version; }

std::map<std::string, std::string> HttpRequest::GetHeaders() const { return headers; }

std::string HttpRequest::GetBody() const { return body; }

std::string HttpRequest::GetErrorMessage() const { return errorMessage; }

ParseState  HttpRequest::getState() const { return state; }

// Setters
void HttpRequest::SetMethod(const std::string& method) { this->method = method; }

void HttpRequest::SetRequestURI(const std::string& requestURI) { this->requestURI = requestURI; }

void HttpRequest::SetVersion(const std::string& version) { this->version = version; }

void HttpRequest::SetHeaders(const std::map<std::string, std::string>& headers) { this->headers = headers; }

void HttpRequest::SetBody(const std::string& body) { this->body = body; }

void HttpRequest::SetErrorMessage(const std::string& errorMessage) { this->errorMessage = errorMessage; }

// Helper Functions
bool HttpRequest::isComplete() const {
    return state == PARSE_DONE;
}

bool HttpRequest::hasError() const {
    return state == PARSE_ERROR;
}
