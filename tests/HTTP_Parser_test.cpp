/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP_Parser_test.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 03:29:50 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 03:36:17 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <vector>
#include "../inc/HTTPRequest.hpp"

void printRequest(const HttpRequest& req) {
	std::cout << "Method: " << req.GetMethod() << std::endl;
	std::cout << "URI: " << req.GetRequestURI() << std::endl;
	std::cout << "Version: " << req.GetVersion() << std::endl;

	std::cout << "\nHeaders:" << std::endl;
	std::map<std::string, std::string> headers = req.GetHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}

	std::cout << "\nBody: ";
	std::string body = req.GetBody();
	if (body.empty())
		std::cout << "(empty)";
	else
		std::cout << "[" << body << "]";
	std::cout << std::endl;
}

int main() {

std::vector<std::pair<std::string, std::string> > tests;

tests.push_back(std::make_pair("Valid GET (working)", 
	"GET /home HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"Connection: close\r\n"
	"\r\n"));

tests.push_back(std::make_pair("Valid POST small body (working)", 
	"POST /submit HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"Content-Length: 10\r\n"
	"\r\n"
	"hello=1234"));

tests.push_back(std::make_pair("Valid POST larger body (working)", 
	"POST /upload HTTP/1.1\r\n"
	"Host: files.example.com\r\n"
	"Content-Length: 26\r\n"
	"Content-Type: text/plain\r\n"
	"\r\n"
	"This is the body content.\n"));

tests.push_back(std::make_pair("Missing Content-Length (POST) (failing)", 
	"POST /form HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"\r\n"
	"data=value"));

tests.push_back(std::make_pair("Invalid Method (failing)", 
	"FETCH /home HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"\r\n"));

tests.push_back(std::make_pair("Invalid HTTP Version (failing)", 
	"GET /home HTTP/2.0\r\n"
	"Host: example.com\r\n"
	"\r\n"));

tests.push_back(std::make_pair("Invalid URI (failing)", 
	"GET badpath HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"\r\n"));

tests.push_back(std::make_pair("Empty Request Line (failing)", 
	"\r\n"
	"Host: example.com\r\n"
	"\r\n"));

tests.push_back(std::make_pair("Malformed Header (no colon) (failing)", 
	"GET /home HTTP/1.1\r\n"
	"Host example.com\r\n"
	"\r\n"));

tests.push_back(std::make_pair("Empty Header Key (failing)", 
	"GET /home HTTP/1.1\r\n"
	": something\r\n"
	"\r\n"));

tests.push_back(std::make_pair("Duplicate Header Field (failing)", 
	"GET /home HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"Host: duplicated.com\r\n"
	"\r\n"));

tests.push_back(std::make_pair("Oversized Body (limit exceeded) (failing)", 
	"POST /upload HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"Content-Length: 2000000\r\n"
	"\r\n"
	"junk"));

tests.push_back(std::make_pair("Body Too Short (failing)", 
	"POST /data HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"Content-Length: 10\r\n"
	"\r\n"
	"short"));

tests.push_back(std::make_pair("Body Longer Than Declared (working)", 
	"POST /pad HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"Content-Length: 5\r\n"
	"\r\n"
	"1234567890"));

tests.push_back(std::make_pair("Minimal GET with no headers (failing)", 
	"GET / HTTP/1.1\r\n"
	"\r\n"));

tests.push_back(std::make_pair("GET with weird casing (working)", 
	"GET /something HTTP/1.1\r\n"
	"ConTent-TyPe: text/html\r\n"
	"Host: test\r\n"
	"\r\n"));


	// Run all tests
	for (size_t i = 0; i < tests.size(); ++i) {
		std::cout << "\n==== Test: " << tests[i].first << " ====\n";
		HttpRequest req;
		ParseStatus status = req.ParseRequest(tests[i].second);
		if (status != Parse_Success)
			std::cerr << "❌ Error: " << req.GetErrorMessage() << "\n";
		else
			printRequest(req);
	}

	return 0;
}
