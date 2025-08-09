/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 00:08:55 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/09 17:59:39 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/HTTPRequest.hpp"
#include "../../inc/helpers/HTTPRequestHelper.hpp"

void HttpRequest::StartNextRequest()
{
	// Reset all member variables to prepare for the next request
	method.clear();
	requestURI.clear();
	version.clear();
	headers.clear();
	body.clear();
	errorMessage.clear();
	state = PARSE_REQUEST_LINE;
}

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
		return (Parse_Incomplete);
	}

	// If we are in the request line state, we need to parse the request line first
	if (state == PARSE_REQUEST_LINE) {
		size_t eol = buffer.find("\r\n");
		if (eol == std::string::npos)
		{
			// NEW: guard against unbounded growth when CRLF hasn't arrived yet
			if (buffer.size() > MAX_START_LINE_BYTES) {
				errorMessage = "Request line too large";
				state = PARSE_ERROR;
				return (Parse_BadRequest); // map to 414 later if desired
			}
			errorMessage = "Request line is incomplete";
			return (Parse_Incomplete);
		}
		// NEW: even if CRLF is present, the start-line might still be too long
		if (eol > MAX_START_LINE_BYTES)
		{
			errorMessage = "Request line too large";
			state = PARSE_ERROR;
			return (Parse_BadRequest); // map to 414 later
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
			// NEW: guard header-section growth before the terminator shows up
			if (buffer.size() > MAX_HEADER_SECTION_BYTES) {
				errorMessage = "Header section too large";
				state = PARSE_ERROR;
				return (Parse_BadRequest); // map to 431 later if desired
			}
			errorMessage = "Headers are incomplete";
			return (Parse_Incomplete);
		}
		// NEW: even if we found the end, reject if section exceeds cap
		if (headerEnd > MAX_HEADER_SECTION_BYTES) {
			errorMessage = "Header section too large";
			state = PARSE_ERROR;
			return (Parse_BadRequest); // map to 431 later
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

		bool hasTransferEncoding = false;
		bool isChunked = false;
		std::map<std::string, std::string>::const_iterator TE = headers.find("transfer-encoding");
		if (TE != headers.end())
		{
			hasTransferEncoding = true;
			std::string transferEncoding = HTTPRequestHelper::toLower(TE->second);

			std::vector<std::string> encodings = HTTPRequestHelper::splitHeaderValue(transferEncoding);
			for (size_t i = 0; i < encodings.size(); ++i)
			{
				if (HTTPRequestHelper::trim(encodings[i]) != "chunked")
				{
					errorMessage = "Unsupported transfer-encoding: " + encodings[i];
					state = PARSE_ERROR;
					return (Parse_NotImplemented);
				}
			}
			isChunked = !encodings.empty();
		}

		bool hasContentLength = headers.find("content-length") != headers.end();

		if (hasTransferEncoding && hasContentLength)
		{
			errorMessage = "Both transfer-encoding and content-length headers are present";
			state = PARSE_ERROR;
			return (Parse_BadRequest);
		}
		if (hasTransferEncoding)
		{
			if (!isChunked)
			{
				errorMessage = "Unsupported transfer-encoding: " + TE->second;
				state = PARSE_ERROR;
				return (Parse_NotImplemented);
			}
			state = PARSE_BODY;
		}
		else if (hasContentLength)
		{
			ParseStatus i = ValidateContentLength();
			if (i != Parse_Success)
			{
				state = PARSE_ERROR;
				return (i);
			}
			size_t contentLength = 0;
			HTTPRequestHelper::parseContentLength(headers["content-length"], contentLength);
			if (contentLength == 0)
			{
				state = PARSE_DONE;
				return (Parse_Success);
			}  
			state = PARSE_BODY;
		}
		else
		{
			state = PARSE_DONE;
			return (Parse_Success);
		}
	}

	// If we are in the body state, we need to parse the body
	if (state == PARSE_BODY)
	{
		if (headers.find("transfer-encoding") != headers.end() &&
			HTTPRequestHelper::toLower(headers["transfer-encoding"]).find("chunked") != std::string::npos)
		{
			ParseStatus status = ParseChunkedBody(buffer);
			if (status != Parse_Success)
			{
				if (status != Parse_Incomplete)
					state = PARSE_ERROR;
				return (status);
			}
			state = PARSE_DONE;
			return (Parse_Success);
		}
		else
		{
			ParseStatus status = ValidateContentLength();
			if (status != Parse_Success)
			{
				state = PARSE_ERROR;
				return (status);
			}
		
			size_t contentLength = 0;
			HTTPRequestHelper::parseContentLength(headers["content-length"], contentLength);
			if (buffer.size() < contentLength)
			{
				errorMessage = "Body is incomplete";
				return (Parse_Incomplete);
			}
		
			status = ParseBody(buffer.substr(0, contentLength));
			if (status != Parse_Success)
			{
				state = PARSE_ERROR;
				return (status);
			}

			buffer.erase(0, contentLength);
		
			state = PARSE_DONE;
			return (Parse_Success);
		}
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

	if (requestComponents[1].size() > MAX_REQUEST_TARGET_BYTES)
	{
		errorMessage = "Request-URI too long";
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
	int headerCount = 0;
	std::string headerLine = HTTPRequestHelper::getHeaderLine(headersRaw, headerIndex);

	std::map<std::string, std::string> headersParsed;
	if (headerLine.empty())
	{
		errorMessage = "No headers found in the request";
		return (Parse_BadRequest);
	}
	while (!headerLine.empty())
	{
		if (++headerCount > MAX_HEADER_COUNT)
		{
			errorMessage = "Too many header fields";
			return (Parse_BadRequest);
		}
		if (headerLine.size() > MAX_HEADER_LINE_BYTES)
		{
			errorMessage = "Header line too large";
			return (Parse_BadRequest);
		}

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
	
	if (version == "HTTP/1.1")
	{
		std::map<std::string, std::string>::const_iterator itHost = headersParsed.find("host");
		if (itHost == headersParsed.end())
		{
			errorMessage = "Missing Host header for HTTP/1.1 request";
			state = PARSE_ERROR;
			return (Parse_BadRequest);
		}
		
		if (HTTPRequestHelper::trim(itHost->second).empty())
		{
			errorMessage = "Empty Host header for HTTP/1.1 request";
			state = PARSE_ERROR;
			return (Parse_BadRequest);
		}
	}
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
	
	size_t contentLength = 0;
	if (!HTTPRequestHelper::parseContentLength(it->second, contentLength))
	{
		errorMessage = "Invalid Content-Length: " + it->second;
		return (Parse_BadRequest);
	}

	// checks if content length does not exceed 1mb
	if (contentLength > maxBodySize)
	{
		std::ostringstream oss;
		oss << "Payload too large: " << contentLength << " bytes exceeds limit";
		errorMessage = oss.str();
		return (Parse_BadRequest);
	}
	
	//checks if the content size is not 0 or negative
	if (contentLength < 1)
	{
		errorMessage = "Invalid Content-Length: " + it->second;
		return (Parse_BadRequest);
	}

	// checks if the content length does not exceed the body
	if (body.size() < contentLength)
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

	size_t contentLength = 0;
	// If Content-Length is not a valid number or is less than 1, return error
	if (!HTTPRequestHelper::parseContentLength(it->second, contentLength))
	{
		errorMessage = "Invalid Content-Length: " + it->second;
		return (Parse_BadRequest);
	}

	// If Content-Length exceeds the maximum allowed size, return error
	if (contentLength > maxBodySize)
	{
		std::ostringstream oss;
		oss << "Payload too large: " << contentLength << " bytes exceeds limit";
		errorMessage = oss.str();
		return (Parse_BadRequest);
	}

	return (Parse_Success);
}

ParseStatus HttpRequest::ParseChunkedBody(const std::string& chunkedBody)
{
	std::string body;
	size_t pos = 0;
	size_t total = 0;

	while (true)
	{
		size_t endOfChunkSize = chunkedBody.find("\r\n", pos);
		if (endOfChunkSize == std::string::npos)
		{
			errorMessage = "Incomplete chunk size line";
			return (Parse_Incomplete);
		}

		std::string chunkSizeStr = chunkedBody.substr(pos, endOfChunkSize - pos);
		size_t semicolon = chunkSizeStr.find(';');
		if (semicolon != std::string::npos)
			chunkSizeStr = chunkSizeStr.substr(0, semicolon);

		if (chunkSizeStr.empty())
		{
			errorMessage = "Invalid chunk size: empty";
			return (Parse_BadRequest);
		}
		char* endp = 0;
		unsigned long v = std::strtoul(chunkSizeStr.c_str(), &endp, 16);
		if (endp == 0 || *endp != '\0')
		{
			errorMessage = "Invalid chunk size: " + chunkSizeStr;
			return (Parse_BadRequest);
		}
		size_t chunkSize = static_cast<size_t>(v);

		pos = endOfChunkSize + 2; // after \r\n

		// Final chunk (0)
		if (chunkSize == 0)
		{
			// OPTION A (simple): reject trailers (only allow immediate CRLF)
			if (chunkedBody.size() < pos + 2)
			{
				errorMessage = "Incomplete end after last chunk";
				return (Parse_Incomplete);
			}
			if (chunkedBody.substr(pos, 2) != "\r\n")
			{
				errorMessage = "Trailers not supported";
				return (Parse_BadRequest);
			}
			pos += 2;

			SetBody(body);
			buffer.erase(0, pos); // consume everything we used
			return (Parse_Success);
		}

		if (chunkedBody.size() < pos + chunkSize + 2) {
			errorMessage = "Incomplete chunk data";
			return (Parse_Incomplete);
		}

		if (maxBodySize - total < chunkSize)
		{
			errorMessage = "Payload too large (chunked)";
			return (Parse_BadRequest);
		}

		// Append chunk
		body.append(chunkedBody, pos, chunkSize);
		total += chunkSize;
		pos += chunkSize;

		// Each chunk must be followed by CRLF
		if (chunkedBody.substr(pos, 2) != "\r\n") {
			errorMessage = "Missing CRLF after chunk data";
			return Parse_BadRequest;
		}
		pos += 2;
	}
}

bool HttpRequest::hasMoreData() const
{
	return (!buffer.empty());
}


// Default constructor
HttpRequest::HttpRequest() : method(), requestURI(), version(), body(), maxBodySize(1 * 1024 * 1024), state(PARSE_REQUEST_LINE) {}

// Constructor with parameters to initialize all components
HttpRequest::HttpRequest(const std::string& method, const std::string& requestURI, const std::string& version, const std::string& body)
	: method(method), requestURI(requestURI), version(version), body(body), maxBodySize(1 * 1024 * 1024), state(PARSE_DONE) {}

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
