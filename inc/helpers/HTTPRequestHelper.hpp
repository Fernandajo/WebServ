/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestHelper.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 00:34:48 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/09 17:33:28 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTHELPER_HPP
#define HTTPREQUESTHELPER_HPP

#include <string>
#include <set>
#include <vector>
#include <limits>
#include <cctype>
#include <sstream>
#include "HTTPRequest.hpp"

#define Whitespace " \t\r\n"

class HTTPRequestHelper
{
	public:
		static bool isValidMethod(const std::string& method);
		static bool isValidRequestURI(const std::string& requestURI);
		static bool isValidVersion(const std::string& version);
		static std::vector<std::string> splitRequestLine(const std::string& line);
		static std::string getHeaderLine(const std::string& rawHeaders, int index);
		
		static std::string trim(const std::string& str);
		static std::string toLower(const std::string& str);

		static bool parseContentLength(const std::string& contentLengthStr, size_t& contentLength);
		static std::vector<std::string> splitHeaderValue(const std::string& headerValue);

		static int MapParseToHttp(const HttpRequest& req, ParseStatus st);
		static const char* ReasonMessage(int errorCode);
		static std::string MakeErrorResponse(int code, const std::string& httpVersion);
};

#endif // HTTPREQUESTHELPER_HPP