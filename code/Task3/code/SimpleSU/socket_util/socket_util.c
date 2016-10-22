/* 
 * File:   socket_util.c
 * Author: Zhuo Zhang, Syracuse University
 *         zzhan38@syr.edu
 *
 * Version: 1.0
 * Release Date: 1/30/2016
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

#include "socket_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>     //socket() bind() listen() accept()
#include <fcntl.h>          //fcntl()
#include <errno.h>          //errno
#include <limits.h>         //PATH_MAX
#include <string.h>

#define LOGE(fmt,args...) fprintf(stderr, fmt, ##args)
#define PLOGE(fmt,args...) LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno))

/*
 * Receive a file descriptor from a Unix socket.
 * Contributed by @mkasick
 *
 * Returns the file descriptor on success, or -1 if a file
 * descriptor was not actually included in the message
 *
 * On error the function terminates by calling exit(-1)
 */
int recv_fd(int sockfd) {
    // Need to receive data from the message, otherwise don't care about it.
    char iovbuf;

    struct iovec iov = {
        .iov_base = &iovbuf,
        .iov_len  = 1,
    };

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    struct msghdr msg = {
        .msg_iov        = &iov,
        .msg_iovlen     = 1,
        .msg_control    = cmsgbuf,
        .msg_controllen = sizeof(cmsgbuf),
    };

    if (recvmsg(sockfd, &msg, MSG_WAITALL) != 1) {
        goto error;
    }

    // Was a control message actually sent?
    switch (msg.msg_controllen) {
    case 0:
        // No, so the file descriptor was closed and won't be used.
        return -1;
    case sizeof(cmsgbuf):
        // Yes, grab the file descriptor from it.
        break;
    default:
        goto error;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

    if (cmsg             == NULL                  ||
        cmsg->cmsg_len   != CMSG_LEN(sizeof(int)) ||
        cmsg->cmsg_level != SOL_SOCKET            ||
        cmsg->cmsg_type  != SCM_RIGHTS) {
error:
        LOGE("unable to read fd");
        exit(-1);
    }

    return *(int *)CMSG_DATA(cmsg);
}

/*
 * Send a file descriptor through a Unix socket.
 * Contributed by @mkasick
 *
 * On error the function terminates by calling exit(-1)
 *
 * fd may be -1, in which case the dummy data is sent,
 * but no control message with the FD is sent.
 */
void send_fd(int sockfd, int fd) {
    // Need to send some data in the message, this will do.
    struct iovec iov = {
        .iov_base = "",
        .iov_len  = 1,
    };

    struct msghdr msg = {
        .msg_iov        = &iov,
        .msg_iovlen     = 1,
    };

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    if (fd != -1) {
        // Is the file descriptor actually open?
        if (fcntl(fd, F_GETFD) == -1) {
            if (errno != EBADF) {
                goto error;
            }
            // It's closed, don't send a control message or sendmsg will EBADF.
        } else {
            // It's open, send the file descriptor in a control message.
            msg.msg_control    = cmsgbuf;
            msg.msg_controllen = sizeof(cmsgbuf);

            struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

            cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
            cmsg->cmsg_level = SOL_SOCKET;
            cmsg->cmsg_type  = SCM_RIGHTS;

            *(int *)CMSG_DATA(cmsg) = fd;
        }
    }

    if (sendmsg(sockfd, &msg, 0) != 1) {
error:
        PLOGE("unable to send fd");
        exit(-1);
    }
}

int read_int(int fd) {
    int val;
    int len = read(fd, &val, sizeof(int));
    if (len != sizeof(int)) {
        LOGE("unable to read int: %d", len);
        exit(-1);
    }
    return val;
}

void write_int(int fd, int val) {
    int written = write(fd, &val, sizeof(int));
    if (written != sizeof(int)) {
        PLOGE("unable to write int");
        exit(-1);
    }
}

char* read_string(int fd) {
    int len = read_int(fd);
    if (len > PATH_MAX || len < 0) {
        LOGE("invalid string length %d", len);
        exit(-1);
    }
    char* val = malloc(sizeof(char) * (len + 1));
    if (val == NULL) {
        LOGE("unable to malloc string");
        exit(-1);
    }
    val[len] = '\0';
    int amount = read(fd, val, len);
    if (amount != len) {
        LOGE("unable to read string");
        exit(-1);
    }
    return val;
}

void write_string(int fd, char* val) {
    int len = strlen(val);
    write_int(fd, len);
    int written = write(fd, val, len);
    if (written != len) {
        PLOGE("unable to write string");
        exit(-1);
    }
}


/*provided by Zhuo Zhang @ Syracuse University*/
//pass dummy message from client to server and wait for response
void handshake_client(int socket) {
    FILE* rand_fp = fopen("/dev/urandom", "r");
    int ack_num;
    fread(&ack_num, sizeof(int), 1, rand_fp);
    fclose(rand_fp);
    
    write_int(socket, ack_num);
    int back_num = read_int(socket);
    
    if (back_num != ack_num) {
        shutdown(socket, SHUT_RDWR);
        close(socket);
        exit(EXIT_FAILURE);
    }
}

/*provided by Zhuo Zhang @ Syracuse University*/
//receive a dummy message from client and send it back
void handshake_server(int socket) {
    int ack_num = read_int(socket);
    write_int(socket, ack_num);
}
