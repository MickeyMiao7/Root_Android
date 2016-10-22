/* 
 * File:   mysu.c
 * Author: Zhuo Zhang, Syracuse University
 *         zzhan38@syr.edu
 *
 * Version: 1.0
 * Release Date: 1/30/2016
 */

/* Version 1.0 - First Release
 * This project is a client
 * It ask the daemon server to launch a root shell for it
 * It will pass its STDIN, STDOUT, STDERR file descriptors to the server via Unix domain socket
 */

 /* This project is based on open source su project
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>     //socket() bind() listen() accept() AF_UNIX
#include <fcntl.h>          //fcntl()
#include <string.h>         //strerror()
#include <errno.h>          //errno
#include <sys/un.h>         //struct sockaddr_un

#include "../socket_util/socket_util.h"
#include "../server_loc.h"

#define ERRMSG(msg) fprintf(stderr, "%s", msg)

//try to connect to the server and get a socket file descriptor
int config_socket() {
    
    struct sockaddr_un sun;
    
    //create socket fd
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        ERRMSG("failed to create socket fd\n");
        exit (EXIT_FAILURE);
    }
    
    //set the socket file descriptor
    //with flag FD_CLOEXEC, socket_fd will stay valid through fork()
    //but will be destroyed by all exec family functions (e.g. execve())
    if (fcntl(socket_fd, F_SETFD, FD_CLOEXEC)) {
        ERRMSG("failed on fcntl\n");
        exit (EXIT_FAILURE);
    }
    
    //set struct sockaddr_un
    /*    
        struct sockaddr_un {
            sa_family_t sun_family;               //AF_UNIX
            char        sun_path[108];            //pathname
        };
    */
    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, SERVER_LOC, sizeof(sun.sun_path));
    
    //connect to server
    if (0 != connect(socket_fd, (struct sockaddr*)&sun, sizeof(sun))) {
        ERRMSG("failed to connect server\n");
        exit (EXIT_FAILURE);
    }
    
    return socket_fd;
}

//try to connect the daemon server
//pass stdin, stdout, stderr to server
//hold the session to operate the root shell created and linked by server
int connect_daemon() {
    
    //get a socket
    int socket = config_socket();
    
    //do handshake
    handshake_client(socket);
    
    send_fd(socket, STDIN_FILENO);      //STDIN_FILENO = 0
    send_fd(socket, STDOUT_FILENO);     //STDOUT_FILENO = 1
    send_fd(socket, STDERR_FILENO);     //STDERR_FILENO = 2
    
    //hold the session until server close the socket or some error occurs
    //in my design, server should not send things back through socket after handshake
    //read() function will block the process, thus we hold the session
    //if the socket is closed, read() will return 0
    //or error occurs, read() will return a negative integer
    char dummy[2];
    int flag = 0;
    do {
        flag = read(socket, &dummy, 1);
    } while (flag > 0);
    
    close(socket);
    
    //print out error message if has
    if (flag < 0) {
        ERRMSG("Socket failed on client: ");
        ERRMSG(strerror(errno));
        ERRMSG("\n");
        return (EXIT_FAILURE);
    }
    
    return (EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    //if not root
    //connect to root daemon for root shell
    if (getuid() != 0 && getgid() != 0) {
        return connect_daemon();
    }
    //if root
    //launch default shell directly
    char* shell[] = {"/bin/sh", NULL};
    execve(shell[0], shell, NULL);
    return (EXIT_SUCCESS);
}

