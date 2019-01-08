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
#ifndef CHERIOS_NGX_OS_H
#define CHERIOS_NGX_OS_H

#include "cheric.h"
#include "nano/nanotypes.h"
#include "cdefs.h"

#define AF_INET         2

typedef size_t rlim_t;


typedef ssize_t (*ngx_recv_pt)(ngx_connection_t *c, u_char *buf, size_t size);
typedef ssize_t (*ngx_recv_chain_pt)(ngx_connection_t *c, ngx_chain_t *in,
                                     off_t limit);
typedef ssize_t (*ngx_send_pt)(ngx_connection_t *c, u_char *buf, size_t size);
typedef ngx_chain_t *(*ngx_send_chain_pt)(ngx_connection_t *c, ngx_chain_t *in,
                                          off_t limit);

#define ngx_cacheline_size  L1_LINE_SIZE

#define ngx_pagesize        UNTRANSLATED_PAGE_SIZE
#define ngx_pagesize_shift  UNTRANSLATED_BITS
#define ngx_debug_init()

#define NGX_IOVS_PREALLOCATE  64

ngx_int_t ngx_os_init(ngx_log_t *log);
void ngx_os_status(ngx_log_t *log);

ngx_int_t ngx_daemon(ngx_log_t *log);


extern ngx_uint_t   ngx_ncpu;
extern ngx_uint_t   ngx_max_wsabufs;
extern ngx_int_t    ngx_max_sockets;

extern char**        environ;

int gethostname(char* name, size_t len);

#define ngx_msleep(t) sleep(MS_TO_CLOCK(t))
#define ngx_cpu_pause(...)
#define ngx_sched_yield(...) sleep(0)

ngx_int_t  ngx_atoi(u_char *line, size_t n);

ngx_int_t ngx_os_signal_process(ngx_cycle_t *cycle, char *sig, ngx_pid_t pid);

int random(void);

ssize_t ngx_unix_recv(ngx_connection_t *c, u_char *buf, size_t size);
ssize_t ngx_readv_chain(ngx_connection_t *c, ngx_chain_t *entry, off_t limit);
ssize_t ngx_udp_unix_recv(ngx_connection_t *c, u_char *buf, size_t size);
ssize_t ngx_unix_send(ngx_connection_t *c, u_char *buf, size_t size);
ngx_chain_t *ngx_writev_chain(ngx_connection_t *c, ngx_chain_t *in,
                              off_t limit);
ssize_t ngx_udp_unix_send(ngx_connection_t *c, u_char *buf, size_t size);
ngx_chain_t *ngx_udp_unix_sendmsg_chain(ngx_connection_t *c, ngx_chain_t *in,
                                        off_t limit);

#if (NGX_HAVE_SENDFILE)
ngx_chain_t * ngx_cherios_sendfile_chain(ngx_connection_t *c, ngx_chain_t *in, off_t limit);
#endif

typedef struct {
    ngx_recv_pt        recv;
    ngx_recv_chain_pt  recv_chain;
    ngx_recv_pt        udp_recv;
    ngx_send_pt        send;
    ngx_send_pt        udp_send;
    ngx_send_chain_pt  udp_send_chain;
    ngx_send_chain_pt  send_chain;
    ngx_uint_t         flags;
} ngx_os_io_t;

typedef struct {
    struct iovec  *iovs;
    ngx_uint_t     count;
    size_t         size;
    ngx_uint_t     nalloc;
} ngx_iovec_t;

ngx_chain_t *ngx_output_chain_to_iovec(ngx_iovec_t *vec, ngx_chain_t *in,
                                       size_t limit, ngx_log_t *log);

ssize_t ngx_writev(ngx_connection_t *c, ngx_iovec_t *vec);

extern ngx_os_io_t  ngx_os_io;
extern ngx_int_t    ngx_max_sockets;
extern ngx_uint_t   ngx_inherited_nonblocking;
extern ngx_uint_t   ngx_tcp_nodelay_and_tcp_nopush;

ngx_int_t ngx_libc_crypt(ngx_pool_t *pool, u_char *key, u_char *salt,
                         u_char **encrypted);

#endif //CHERIOS_NGX_OS_H
