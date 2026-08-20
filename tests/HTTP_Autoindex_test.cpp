/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP_Autoindex_test.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mdomnik <mdomnik@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 23:28:35 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/29 23:58:24 by mdomnik          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// tests/HTTP_Autoindex_test.cpp

#include "../inc/HTTPResponse.hpp"
#include "../inc/HTTPRequest.hpp"
#include "../inc/Server.hpp"
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>


void test_nested_directory_listing()
{
	std::cout << "\n===== Test Autoindex Nested Directory =====" << std::endl;

	// Simulate a GET request to /testdir/nested/
	HttpRequest request;
	request.SetMethod("GET");
	request.SetRequestURI("/testdir/nested/");
	request.SetVersion("HTTP/1.1");

	// Setup routing with autoindex enabled
	RoutingConfig route;
	route.path = "/testdir";
	route.root = "tests/testroot";
	route.isAutoIndexOn = true;
	route.methods.push_back("GET");

	ServerConfig server;
	server.root = "tests/testroot";
	server.routes.push_back(route);

	// Generate the response
	HTTPResponse response;
	std::string httpResponse = response.GenerateResponse(request, server);
	std::cout << httpResponse << std::endl;

	if (httpResponse.find("nestedfile.txt") != std::string::npos)
		std::cout << "✅ Nested directory listing successful!\n";
	else
		std::cout << "❌ Nested directory file missing.\n";
}

void test_directory_link_back()
{
	std::cout << "\n===== Test Parent Directory Link =====" << std::endl;

	HttpRequest request;
	request.SetMethod("GET");
	request.SetRequestURI("/testdir/nested/");
	request.SetVersion("HTTP/1.1");

	RoutingConfig route;
	route.path = "/testdir";
	route.root = "tests/testroot";
	route.isAutoIndexOn = true;
	route.methods.push_back("GET");

	ServerConfig server;
	server.root = "tests/testroot";
	server.routes.push_back(route);

	HTTPResponse response;
	std::string httpResponse = response.GenerateResponse(request, server);
	std::cout << httpResponse << std::endl;

	if (httpResponse.find("<a href=\"/testdir/") != std::string::npos)
		std::cout << "✅ Parent directory link exists!\n";
	else
		std::cout << "❌ Parent directory link missing.\n";
}


void test_autoindex_enabled()
{
	std::cout << "\n===== Test Autoindex Enabled =====" << std::endl;

	// Create test directory and files
	system("mkdir -p www/testdir");
	system("touch www/testdir/file1.txt");
	system("touch www/testdir/file2.txt");

	// Prepare request
	HttpRequest request;
	request.SetMethod("GET");
	request.SetRequestURI("/testdir/");
	request.SetVersion("HTTP/1.1");

	// Prepare server config
	ServerConfig config;
	config.root = "www";

	RoutingConfig route;
	route.path = "/";
	route.root = "www";
	route.isAutoIndexOn = true;
	route.indexFile = ""; // no index
	route.methods.push_back("GET");

	config.routes.push_back(route);

	// Generate response
	HTTPResponse resp;
	std::string response = resp.GenerateResponse(request, config);
	std::cout << response << std::endl;

	if (response.find("Index of /testdir/") != std::string::npos)
		std::cout << "✅ Autoindex page generated!" << std::endl;
	else
		std::cout << "❌ Autoindex failed." << std::endl;
}

void test_autoindex_disabled()
{
	std::cout << "\n===== Test Autoindex Disabled =====" << std::endl;

	// Ensure same test directory is used
	system("mkdir -p www/testdir");
	system("touch www/testdir/file1.txt");

	// Prepare request
	HttpRequest request;
	request.SetMethod("GET");
	request.SetRequestURI("/testdir/");
	request.SetVersion("HTTP/1.1");

	// Prepare server config
	ServerConfig config;
	config.root = "www";

	RoutingConfig route;
	route.path = "/";
	route.root = "www";
	route.isAutoIndexOn = false;
	route.indexFile = ""; // no index
	route.methods.push_back("GET");

	config.routes.push_back(route);

	// Generate response
	HTTPResponse resp;
	std::string response = resp.GenerateResponse(request, config);
	std::cout << response << std::endl;

	if (response.find("Index of /testdir/") == std::string::npos &&
		response.find("403 Forbidden") != std::string::npos)
		std::cout << "✅ Forbidden response as expected!" << std::endl;
	else
		std::cout << "❌ Autoindex was incorrectly enabled or fallback failed." << std::endl;
}

int main()
{
	test_autoindex_enabled();
	test_autoindex_disabled();
	test_nested_directory_listing();
	test_directory_link_back();
	return 0;
}

