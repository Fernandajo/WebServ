/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/24 23:58:52 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 03:21:44 by mdomnik          ###   ########.fr       */
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

enum ParseStatus
{
	Parse_Success,
	Parse_BadRequest,
	Parse_NotImplemented,
	Parse_VersionNotSupported
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

	public:
	
		//getters
		std::string GetMethod() const;
		std::string GetRequestURI() const;
		std::string GetVersion() const;
		std::map<std::string, std::string> GetHeaders() const;
		std::string GetBody() const;

		std::string GetErrorMessage() const;

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
		ParseStatus ParseRequestLine(const std::string& requestLine);
		ParseStatus ParseHeaders(const std::string& headers);
		ParseStatus ParseBody(const std::string& body);

		ParseStatus ParseRequest(const std::string& raw);
};

#endif