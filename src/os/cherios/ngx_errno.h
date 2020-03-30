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
#ifndef CHERIOS_NGX_ERRNO_H
#define CHERIOS_NGX_ERRNO_H

#include "cheric.h"
#include "string_enums.h"
#include "errno.h"
#include "sockets.h"
#include "unistd.h"

typedef int ngx_err_t;

ngx_int_t ngx_strerror_init(void);

#define NGX_FILE_ERROR  NGX_ERROR

#define NGX_APPEND(x) NGX_ ## x = x,

enum ngx_err_e {
    ER_LIST(NGX_APPEND)
};

#define NGX_EEXIST_FILE     NGX_EEXIST
#define NGX_ENOMOREFILES    NGX_EEXIST
#define NGX_ENOENT          NGX_EEXIST
#define NGX_ENFILE          NGX_EMFILE
#define NGX_ENOSYS          NGX_EOPNOTSUPP

u_char * ngx_strerror(ngx_err_t err, u_char *errstr, size_t size);

#define ngx_errno                  errno
#define ngx_socket_errno           errno
#define ngx_set_errno(err)         errno = err
#define ngx_set_socket_errno(err)  errno = err

#endif //CHERIOS_NGX_ERRNO_H
