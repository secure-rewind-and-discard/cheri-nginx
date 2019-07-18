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

#include <ngx_config.h>
#include <ngx_core.h>
#include <nginx.h>
#include "hostconfig.h"


ngx_os_io_t ngx_os_io = {
        ngx_unix_recv,                   // recv
        ngx_readv_chain,            // readv_chain
        ngx_udp_unix_recv,          // udp_recv
        ngx_unix_send,              // send
        ngx_udp_unix_send,          // udp_send
        ngx_udp_unix_sendmsg_chain, // udp_sendmsg_chain
#if (NGX_HAVE_SENDFILE)
        ngx_cherios_sendfile_chain, // write
        NGX_IO_SENDFILE             // flags
#else
        ngx_writev_chain,           // writev
        0                           // flags
#endif
};

ngx_uint_t  ngx_ncpu = SMP_CORES;
ngx_uint_t  ngx_inherited_nonblocking = 1;
ngx_uint_t  ngx_tcp_nodelay_and_tcp_nopush = 1;

// TODO I have no idea what these are
ngx_uint_t   ngx_max_wsabufs = 64;
ngx_int_t    ngx_max_sockets = 64;

char* environ_default[] = {NULL};
char**        environ = environ_default;

ngx_int_t ngx_os_init(ngx_log_t *log) {
    while(try_get_fs() == NULL) sleep(0);

    int res;

#define Tmkdir(X) res = mkdir(X) // if(res != 0) printf("[nginx] could not make dir" X " er = %d\n", res)
    // These may not exist so create them just in case
    Tmkdir("/nginx");
    Tmkdir("/nginx/bin");
    Tmkdir("/nginx/conf");
    Tmkdir("/nginx/logs");
    Tmkdir("/nginx/modules");
    Tmkdir("/nginx/temp");

    res = unlink(NGX_PREFIX NGX_HTTP_CLIENT_TEMP_PATH);

    msg_allow_more_sends();

    return NGX_OK;
}

void ngx_os_status(ngx_log_t *log) {

}

ngx_uint_t             ngx_event_flags;

int gethostname(char* name, size_t len) {
    if(sizeof(CHERIOS_HOST) > len) return -1;
    memcpy(name, CHERIOS_HOST, sizeof(CHERIOS_HOST));
    return 0;
}

int random(void) {
    return rand();
}

ngx_int_t ngx_libc_crypt(ngx_pool_t *pool, u_char *key, u_char *salt,
                         u_char **encrypted) {
    assert(0 && "No libc crypt =(");
    while(1);
}