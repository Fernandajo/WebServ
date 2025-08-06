/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse_helper.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/04 00:39:02 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/04 00:45:24 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../HTTPResponseHelper.hpp"

std::string HTTPResponseHelper::urlDecode(const std::string& str)
{
	std::ostringstream decoded;
	for (size_t i = 0; i < str.size(); ++i) {
		if (str[i] == '%' && i + 2 < str.size()) {
			std::string hex = str.substr(i + 1, 2);
			char ch = static_cast<char>(std::stoi(hex, nullptr, 16));
			decoded << ch;
			i += 2;
		} else if (str[i] == '+') {
			decoded << ' ';
		} else {
			decoded << str[i];
		}
	}
	return decoded.str();
}
