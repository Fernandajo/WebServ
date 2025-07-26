/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjoestin <fjoestin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 18:13:22 by mdomnik           #+#    #+#             */
/*   Updated: 2025/07/26 16:34:26 by fjoestin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HELPERS_HPP
# define HELPERS_HPP

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <cstring>
#include <netdb.h>
#include <signal.h>
#include <ctime>
#include <cstdlib>

void set_nonblocking(int fd);
std::string GetMimeType(const std::string& path);

#endif