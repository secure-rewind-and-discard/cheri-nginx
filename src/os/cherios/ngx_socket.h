/*-
 * Copyright (c) 2018 Lawrence Esswood
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract FA8750-10-C-0237
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifndef CHERIOS_NGX_SOCKET_H
#define CHERIOS_NGX_SOCKET_H

#include "stdio.h"
#include "unistd.h"
#include "net.h"

typedef unix_like_socket* ngx_socket_t;

#define NGX_NO_POLL (ngx_socket_t)NULL

typedef uint16_t in_port_t;

#define IP_BIND_ADDRESS_NO_PORT 0
#define AF_UNIX 1
#define NGX_LISTEN_BACKLOG 16

int getsockopt(ngx_socket_t sockfd, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(ngx_socket_t sockfd, int level, int optname, const void *optval, socklen_t optlen);
int getsockname(ngx_socket_t sockfd, struct sockaddr *addr, socklen_t *addrlen);

#define TCP_FASTOPEN 0x06

#define SOCK_NONBLOCK MSG_DONT_WAIT

ngx_socket_t ngx_socket(int domain, int type, int protocol);
#define ngx_socket_n "SOCKET()"

int ngx_close_socket(ngx_socket_t s);
//#define ngx_close_socket(s) close(s)
#define ngx_close_socket_n "CLOSE_SOCKET()"

ngx_err_t ngx_nonblocking(ngx_socket_t s);
#define ngx_nonblocking_n "NON_BLOCKING()"

ngx_err_t ngx_blocking(ngx_socket_t s);
#define ngx_blocking_n "BLOCKING()"

size_t ngx_sock_to_int(ngx_socket_t sockfd);

int ngx_tcp_push(ngx_socket_t s);
#define ngx_tcp_push_n            "tcp_push()"

int ngx_shutdown_socket(ngx_socket_t s, int how);
#define ngx_shutdown_socket_n  "shutdown()"

#define NGX_WRITE_SHUTDOWN SHUT_WR

#endif //CHERIOS_NGX_SOCKET_H
