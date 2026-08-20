/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP_Response_test.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: moojig12 <moojig12@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 18:31:09 by mdomnik           #+#    #+#             */
/*   Updated: 2025/08/04 00:49:04 by moojig12         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "../inc/HTTPResponse.hpp"
#include "../inc/HTTPRequest.hpp"
#include "../inc/Server.hpp"
#include "../inc/MultiServerManager.hpp"

void createTestFile(const std::string& path, const std::string& content) {
	std::ofstream out(path.c_str(), std::ios::out | std::ios::binary);
	out << content;
	out.close();
}

void removeTestFile(const std::string& path) {
	std::remove(path.c_str());
}

void testGETExistingFile() {
	std::string filePath = "./www/test_get.html";
	createTestFile(filePath, "<html>GET OK</html>");

	HttpRequest req("GET", "/test_get.html", "HTTP/1.1", "");
	HTTPResponse res;
	std::string response = res.GenerateResponse(req);

	std::cout << "[GET existing] ";
	if (response.find("200 OK") != std::string::npos &&
		response.find("GET OK") != std::string::npos)
		std::cout << "✔ Passed\n";
	else
		std::cout << "✘ Failed\n";

	removeTestFile(filePath);
}

void testGETMissingFile() {
	HttpRequest req("GET", "/nonexistent.html", "HTTP/1.1", "");
	HTTPResponse res;
	std::string response = res.GenerateResponse(req);

	std::cout << "[GET missing] ";
	if (response.find("404 Not Found") != std::string::npos)
		std::cout << "✔ Passed\n";
	else
		std::cout << "✘ Failed\n";
}

void testPOSTUpload() {
	HttpRequest req("POST", "/uploads/test_upload.txt", "HTTP/1.1", "Upload test content");
	HTTPResponse res;
	std::string response = res.GenerateResponse(req);

	std::cout << "[POST upload] ";
	if (response.find("201 Created") != std::string::npos)
		std::cout << "✔ Passed\n";
	else
		std::cout << "✘ Failed\n";
}

void testDELETEUploadedFile() {
	std::string filePath = "./www/uploads/test_delete.txt";
	createTestFile(filePath, "To be deleted");

	HttpRequest req("DELETE", "/uploads/test_delete.txt", "HTTP/1.1", "");
	HTTPResponse res;
	std::string response = res.GenerateResponse(req);

	std::cout << "[DELETE file] ";
	if (response.find("200 OK") != std::string::npos ||
		response.find("204") != std::string::npos)
		std::cout << "✔ Passed\n";
	else
		std::cout << "✘ Failed\n";
}

void testUnsupportedMethod() {
	HttpRequest req("PUT", "/something", "HTTP/1.1", "");
	HTTPResponse res;
	std::string response = res.GenerateResponse(req);

	std::cout << "[PUT unsupported] ";
	if (response.find("405 Method Not Allowed") != std::string::npos)
		std::cout << "✔ Passed\n";
	else
		std::cout << "✘ Failed\n";
}

void testEmptyPOSTBody() {
	HttpRequest req("POST", "/uploads/test_empty.txt", "HTTP/1.1", "");
	HTTPResponse res;
	std::string response = res.GenerateResponse(req);

	std::cout << "[POST empty body] ";
	if (response.find("400 Bad Request") != std::string::npos)
		std::cout << "✔ Passed\n";
	else
		std::cout << "✘ Failed\n";
}

void testDeleteMissingFile() {
	HttpRequest req("DELETE", "/uploads/missing.txt", "HTTP/1.1", "");
	HTTPResponse res;
	std::string response = res.GenerateResponse(req, );

	std::cout << "[DELETE missing] ";
	if (response.find("404 Not Found") != std::string::npos)
		std::cout << "✔ Passed\n";
	else
		std::cout << "✘ Failed\n";
}


int main(int argc, char* argv[]) {
	std::cout << "=== HTTPResponse Tests ===\n";
	if(argc == 2) {
		std::string configFile = argv[1];
		// parsing of config file can be added here
		MultiServerManager serverManager(configFile);
	} else {
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return 1;
	}
	return 0;


	std::cout << "==========================\n";
}

