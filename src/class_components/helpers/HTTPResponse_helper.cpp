/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse_helper.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/04 00:39:02 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/11 20:37:40 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../inc/helpers/HTTPResponseHelper.hpp"

static int hexVal(char c)
{
	if (c >= '0' && c <= '9')
		return c - 48;
	if (c >= 'A' && c <= 'F')
		return c - 65 + 10;
	if (c >= 'a' && c <= 'f')
		return c - 97 + 10;
	return -1;
}

std::string HTTPResponseHelper::urlDecode(const std::string& str)
{
	std::string output;
	output.reserve(str.size());
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] == '%' && i + 2 < str.size())
		{
			int high = hexVal(str[i + 1]);
			int low = hexVal(str[i + 2]);
			if (high >= 0 && low >= 0)
			{
				output.push_back(char((high << 4)|low));
				i += 2;
			}
			else
				output.push_back(str[i]);
		}
		else
			output += str[i];
	}
	return output;
}

std::string HTTPResponseHelper::extractKeyValue(const std::string& str, const std::string& key)
{
	size_t pos = 0;
	while (pos <= str.size())
	{
		size_t sign = str.find('&', pos);
		std::string pair = (str.substr(pos, sign == std::string::npos ? std::string::npos : sign - pos));
		size_t equalPos = pair.find('=');
		if (equalPos != std::string::npos)
		{
			std::string keyPart = pair.substr(0, equalPos);
			if (keyPart == key)
				return (urlDecode(pair.substr(equalPos + 1)));
		}
		if (sign == std::string::npos)
			break;
		pos = sign + 1;
	}
	return "";
}
