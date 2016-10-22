/* 
 * File:   socket_util.h
 * Author: Zhuo Zhang, Syracuse University
 *         zzhan38@syr.edu
 *
 * Version: 1.0
 * Release Date: 1/30/2016
 */

/*
 * This is a static library with some useful function for sockets:
 * int recv_fd(int sockfd)
 * void send_fd(int sockfd, int fd)
 * int read_int(int fd)
 * void write_int(int fd, int val)
 * char* read_string(int fd)
 * void write_string(int fd, char* val)
 * -------------------------------------
 * void handshake_client(int socket)
 * void handshake_server(int socket)
 * 
 * NOTE:
 * Two handshake functions are only for synchronization before message passing
 * No security promise are made
 * 
 * functions above line are from open source su project
 * Source: https://github.com/koush/Superuser
 * Original License:
 *   ** Copyright 2010, Adam Shanks (@ChainsDD)
 *   ** Copyright 2008, Zinx Verituse (@zinxv)
 *   **
 *   ** Licensed under the Apache License, Version 2.0 (the "License");
 *   ** you may not use this file except in compliance with the License.
 *   ** You may obtain a copy of the License at
 *   **
 *   **     http://www.apache.org/licenses/LICENSE-2.0
 *   **
 *   ** Unless required by applicable law or agreed to in writing, software
 *   ** distributed under the License is distributed on an "AS IS" BASIS,
 *   ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   ** See the License for the specific language governing permissions and
 *   ** limitations under the License.
 */

#ifndef SOCKET_UTIL_H
#define SOCKET_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Receive a file descriptor from a Unix socket.
 * Contributed by @mkasick
 *
 * Returns the file descriptor on success, or -1 if a file
 * descriptor was not actually included in the message
 *
 * On error the function terminates by calling exit(-1)
 */
int recv_fd(int sockfd);

/*
 * Send a file descriptor through a Unix socket.
 * Contributed by @mkasick
 *
 * On error the function terminates by calling exit(-1)
 *
 * fd may be -1, in which case the dummy data is sent,
 * but no control message with the FD is sent.
 */
void send_fd(int sockfd, int fd);

int read_int(int fd);

void write_int(int fd, int val);

char* read_string(int fd);

void write_string(int fd, char* val);

void handshake_client(int socket);

void handshake_server(int socket);

#ifdef __cplusplus
}
#endif

#endif /* SOCKET_UTIL_H */

