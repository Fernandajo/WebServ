/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponseHelper.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/04 00:39:41 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/11 20:28:34 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSEHELPER_HPP
#define HTTPRESPONSEHELPER_HPP

#include <sstream>
#include <iomanip>
#include <string>

class HTTPResponseHelper
{
	public:
		static std::string urlDecode(const std::string& str);
		static std::string extractKeyValue(const std::string& str, const std::string& key);
};

#endif