/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestHelper.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 00:34:48 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 02:31:17 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTHELPER_HPP
#define HTTPREQUESTHELPER_HPP

#include <string>
#include <set>
#include <vector>

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
};

#endif // HTTPREQUESTHELPER_HPP