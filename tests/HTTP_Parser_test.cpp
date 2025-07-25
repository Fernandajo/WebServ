/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP_Parser_test.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 04:11:49 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/25 04:49:07 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include "../inc/HTTPRequest.hpp"

const char* getStateName(ParseState s) {
	switch (s) {
		case PARSE_REQUEST_LINE: return "PARSE_REQUEST_LINE";
		case PARSE_HEADERS:      return "PARSE_HEADERS";
		case PARSE_BODY:         return "PARSE_BODY";
		case PARSE_DONE:         return "PARSE_DONE";
		case PARSE_ERROR:        return "PARSE_ERROR";
		default:                 return "Unknown";
	}
}

const char* getStatusName(ParseStatus s) {
	switch (s) {
		case Parse_Success:         return "Parse_Success";
		case Parse_BadRequest:      return "Parse_BadRequest";
		case Parse_NotImplemented:  return "Parse_NotImplemented";
		case Parse_VersionNotSupported: return "Parse_VersionNotSupported";
		case Parse_Incomplete:      return "Parse_Incomplete";
		default:                    return "Unknown";
	}
}

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

void runTests() {
	std::vector<std::pair<std::string, std::string> > tests;

	tests.push_back(std::make_pair("Valid GET (working)",
		"GET /home HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"));
	tests.push_back(std::make_pair("Valid POST small body (working)",
		"POST /submit HTTP/1.1\r\nHost: example.com\r\nContent-Length: 10\r\n\r\nhello=1234"));
	tests.push_back(std::make_pair("Valid POST larger body (working)",
		"POST /upload HTTP/1.1\r\nHost: files.example.com\r\nContent-Length: 26\r\nContent-Type: text/plain\r\n\r\nThis is the body content.\n"));
	tests.push_back(std::make_pair("Missing Content-Length (POST) (failing)",
		"POST /form HTTP/1.1\r\nHost: example.com\r\n\r\ndata=value"));
	tests.push_back(std::make_pair("Invalid Method (failing)",
		"FETCH /home HTTP/1.1\r\nHost: example.com\r\n\r\n"));
	tests.push_back(std::make_pair("Invalid HTTP Version (failing)",
		"GET /home HTTP/2.0\r\nHost: example.com\r\n\r\n"));
	tests.push_back(std::make_pair("Invalid URI (failing)",
		"GET badpath HTTP/1.1\r\nHost: example.com\r\n\r\n"));
	tests.push_back(std::make_pair("Empty Request Line (failing)",
		"\r\nHost: example.com\r\n\r\n"));
	tests.push_back(std::make_pair("Malformed Header (no colon) (failing)",
		"GET /home HTTP/1.1\r\nHost example.com\r\n\r\n"));
	tests.push_back(std::make_pair("Empty Header Key (failing)",
		"GET /home HTTP/1.1\r\n: something\r\n\r\n"));
	tests.push_back(std::make_pair("Duplicate Header Field (failing)",
		"GET /home HTTP/1.1\r\nHost: example.com\r\nHost: duplicated.com\r\n\r\n"));
	tests.push_back(std::make_pair("Oversized Body (limit exceeded) (failing)",
		"POST /upload HTTP/1.1\r\nHost: example.com\r\nContent-Length: 2000000\r\n\r\njunk"));
	tests.push_back(std::make_pair("Body Too Short (failing)",
		"POST /data HTTP/1.1\r\nHost: example.com\r\nContent-Length: 10\r\n\r\nshort"));
	tests.push_back(std::make_pair("Body Longer Than Declared (working)",
		"POST /pad HTTP/1.1\r\nHost: example.com\r\nContent-Length: 5\r\n\r\n1234567890"));
	tests.push_back(std::make_pair("Minimal GET with no headers (failing)",
		"GET / HTTP/1.1\r\n\r\n"));
	tests.push_back(std::make_pair("GET with weird casing (working)",
		"GET /something HTTP/1.1\r\nConTent-TyPe: text/html\r\nHost: test\r\n\r\n"));

	for (size_t i = 0; i < tests.size(); ++i) {
		std::cout << "\n==== Test: " << tests[i].first << " ====" << std::endl;
		HttpRequest req;
		ParseStatus status = req.ParseRequestChunk(tests[i].second);
		std::cout << "Status: " << getStatusName(status) << std::endl;
		std::cout << "Parser State: " << getStateName(req.getState()) << std::endl;

		if (status != Parse_Success || !req.isComplete())
			std::cerr << "❌ Error: " << req.GetErrorMessage() << std::endl;
		else
			printRequest(req);
	}
}

void runChunkedTest() {
	std::cout << "\n==== Test: Chunked Request Parsing (working) ====" << std::endl;

	std::vector<std::string> chunks;
	chunks.push_back("POST /submit HTTP/1.1\r\nHost: examp");
	chunks.push_back("le.com\r\nContent-Length: 10\r\n\r\n");
	chunks.push_back("hello=1234");

	HttpRequest req;
	for (size_t i = 0; i < chunks.size(); ++i) {
		ParseStatus status = req.ParseRequestChunk(chunks[i]);
		std::cout << "[Chunk " << i + 1 << "] Status: " << getStatusName(status)
		          << ", State: " << getStateName(req.getState()) << std::endl;

		if (status != Parse_Success && status != Parse_Incomplete) {
			std::cerr << "❌ Error: " << req.GetErrorMessage() << std::endl;
			return;
		}
		if (req.hasError()) {
			std::cerr << "❌ Error: " << req.GetErrorMessage() << std::endl;
			return;
		}
		if (req.isComplete()) {
			printRequest(req);
			return;
		}
	}
	std::cerr << "❌ Chunked parsing did not complete!" << std::endl;
}

int main() {
	runTests();
	runChunkedTest();
	return 0;
}
