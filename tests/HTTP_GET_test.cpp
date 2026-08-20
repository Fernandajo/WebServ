/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP_GET_test.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 05:06:55 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 05:18:21 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "../inc/HTTPRequest.hpp"

void simulateGET(const std::string& raw) {
	HttpRequest req;
	ParseStatus status = req.ParseRequestChunk(raw);

	std::cout << "Status: " << status << std::endl;
	std::cout << "Parser State: " << req.getState() << std::endl;

	if (status == Parse_Success && req.isComplete()) {
		if (req.GetMethod() == "GET") {
			std::cout << "Early GET Response:\n";
			std::cout << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, world!\n";
		}
	} else {
		std::cerr << "❌ Error: " << req.GetErrorMessage() << std::endl;
	}
}

int main() {
	std::string raw =
		"GET /hello HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"\r\n";

	std::cout << "==== Test: Early GET Handling ====" << std::endl;
	simulateGET(raw);
	return 0;
}
