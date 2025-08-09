/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest_helper.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 00:33:19 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/09 18:02:40 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../inc/helpers/HTTPRequestHelper.hpp"

// Compares input method name against a set of predefined names
// returns 0 on success and 1 on failure
bool HTTPRequestHelper::isValidMethod(const std::string& method)
{
	static std::set<std::string> validMethods;

	if (validMethods.empty())
	{
		validMethods.insert("GET");
		validMethods.insert("POST");
		validMethods.insert("DELETE");
	}

	return (validMethods.find(method) != validMethods.end());
}

// Compares input request URI against a set of predefined rules
// returns 0 on success and 1 on failure
bool HTTPRequestHelper::isValidRequestURI(const std::string& requestURI)
{
	// Check if the request URI is empty or does not start with '/'
	if (requestURI.empty() || requestURI[0] != '/')
		return (false);

	// prevent directory traversal attacks
	if (requestURI.find("..") != std::string::npos)
		return (false);

	// rejects any non-printable characters
	for (size_t i = 0; i < requestURI.size(); ++i) {
		if (requestURI[i] < 32 || requestURI[i] > 126)
			return (false);
	}
	return (true);
}

// Compares input version against a set of predefined versions
// returns 0 on success and 1 on failure
bool HTTPRequestHelper::isValidVersion(const std::string& version)
{
	static std::set<std::string> validVersions;

	if (validVersions.empty())
	{
		validVersions.insert("HTTP/1.1");
		validVersions.insert("HTTP/1.0");
	}

	return (validVersions.find(version) != validVersions.end());
}

// Removes any whitespaces before or after a line of characters
std::string HTTPRequestHelper::trim(const std::string& str)
{
	size_t start = str.find_first_not_of(Whitespace);
	size_t end = str.find_last_not_of(Whitespace);

	if (start == std::string::npos || end == std::string::npos)
		return ("");
	return (str.substr(start, end - start + 1));
}

// Splits the request line by whitespaces and return a vector of string pointing to
// [0[method, [1]Request-URI, [2]HTTP-Version 
std::vector<std::string> HTTPRequestHelper::splitRequestLine(const std::string& line)
{
	std::vector<std::string> parts;

	//finds the first delimiter
	size_t delim1 = line.find(' ');
	if (delim1 == std::string::npos)
		return (parts);
	
	//finds the second delimiter
	size_t delim2 = line.find(' ', delim1 + 1);
	if (delim2 == std::string::npos)
		return (parts);
	
	//splits the string by parts
	std::string method = line.substr(0, delim1);
	std::string requestURI = line.substr(delim1 + 1, delim2 - delim1 - 1);
	std::string version = line.substr(delim2 + 1);

	//checks if all components are filled
	method = trim(method);
	requestURI = trim(requestURI);
	version = trim(version);

	if (method.empty() || requestURI.empty() || version.empty())
		return (parts);
	
	//populates the parts vector container on successfull read
	parts.push_back(method);
	parts.push_back(requestURI);
	parts.push_back(version);
	
	return (parts);
	
}

// Returns the header line at the specified index from the raw headers string
std::string HTTPRequestHelper::getHeaderLine(const std::string& rawHeaders, int index)
{
	size_t start = 0;
	int internalIterator = 0;

	// Iterate through the raw headers to find the specifically indexed header
	while (start < rawHeaders.length())
	{
		size_t end = rawHeaders.find("\r\n", start);
		if (end == std::string::npos)
			end = rawHeaders.length();
		if (internalIterator == index)
		{
			return rawHeaders.substr(start, end - start);
		}
		start = end + 2; // Move past the "\r\n"
		internalIterator++;
	}
	return ("");
}

std::string HTTPRequestHelper::toLower(const std::string& str)
{
	std::string lower = str;
	for (size_t i = 0; i < lower.size(); ++i)
	{
		if (lower[i] >= 'A' && lower[i] <= 'Z')
			lower[i] += ('a' - 'A');
	}
	return (lower);
}

bool HTTPRequestHelper::parseContentLength(const std::string& contentLengthStr, size_t& contentLength)
{
	if (contentLengthStr.empty())
		return (false);
	unsigned long long v = 0;
	for (size_t i = 0; i < contentLengthStr.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(contentLengthStr[i])))
			return (false); // Invalid character in Content-Length
		v = v * 10 + (contentLengthStr[i] - '0');
		if (v > static_cast<unsigned long long>(std::numeric_limits<size_t>::max()))
			return (false);
	}
	contentLength = static_cast<size_t>(v);
	return (true);
}

std::vector<std::string> HTTPRequestHelper::splitHeaderValue(const std::string& headerValue)
{
	std::vector<std::string> values;
	size_t start = 0;
	size_t end = 0;

	while ((end = headerValue.find(',', start)) != std::string::npos)
	{
		std::string value = trim(headerValue.substr(start, end - start));
		if (!value.empty())
			values.push_back(value);
		start = end + 1;
	}

	std::string value = trim(headerValue.substr(start));
	if (!value.empty())
		values.push_back(value);

	return (values);
}

int HTTPRequestHelper::MapParseToHttp(const HttpRequest& req, ParseStatus st)
{
	if (st == Parse_Incomplete) return 0;
	if (st == Parse_NotImplemented) return 501;
	if (st == Parse_VersionNotSupported) return 505;
	if (st == Parse_Success) return 200;
	
	const std::string& errorMessage = req.GetErrorMessage();

	if (errorMessage.find("Payload too large") != std::string::npos)
		return 413;
	if (errorMessage.find("Request-URI too long") != std::string::npos)
		return 414;
	if (errorMessage.find("Request line too large") != std::string::npos)
		return 414;
	if (errorMessage.find("Header section too large") != std::string::npos)
		return 431;
	if (errorMessage.find("Header line too large") != std::string::npos)
		return 431;
	if (errorMessage.find("Too many header fields") != std::string::npos)
		return 431;

	return 400;
}

const char* HTTPRequestHelper::ReasonMessage(int errorCode)
{
	switch (errorCode)
	{
		case 400: return "Bad Request";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 431: return "Request Header Fields Too Large";
		case 501: return "Not Implemented";
		case 505: return "HTTP Version Not Supported";
		default: return "Unknown Error";
	}
}

std::string HTTPRequestHelper::MakeErrorResponse(int code, const std::string& httpVersion)
{
	const char* reason = HTTPRequestHelper::ReasonMessage(code);
	std::string body = std::string(reason) + "\n";
	std::ostringstream oss;
	oss << httpVersion << " " << code << " " << reason << "\r\n"
		<< "Content-Type: text/plain\r\n"
		<< "Content-Length: " << body.size() << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n"
		<< body;
	return (oss.str());
}