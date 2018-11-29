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


// TODO map some sock errors

// The FR errors are exactly aligned with NGX errors so we can cast

#define NGX_ER_LIST(ITEM)               \
    ITEM(NGX_NO_ER = 0)                 \
    ITEM(NGX_FR_DISK_ERR)               \
    ITEM(NGX_FR_INT_ERR)                \
    ITEM(NGX_FR_NOT_READY)              \
    ITEM(NGX_EEXIST)                    \
    ITEM(NGX_ENOPATH)                   \
    ITEM(NGX_ENAMETOOLONG)              \
    ITEM(NGX_EACCES)                    \
    ITEM(NGX_FR_EXIST)                  \
    ITEM(NGX_FR_INVALID_OBJECT)         \
    ITEM(NGX_FR_WRITE_PROTECTED)        \
    ITEM(NGX_FR_INVALID_DRIVE)          \
    ITEM(NGX_FR_NOT_ENABLED)            \
    ITEM(NGX_FR_NO_FILESYSTEM)          \
    ITEM(NGX_FR_MKFS_ABORTED)           \
    ITEM(NGX_FR_TIMEOUT)                \
    ITEM(NGX_FR_LOCKED)                 \
    ITEM(NGX_FR_NOT_ENOUGH_CORE)        \
    ITEM(NGX_EMFILE)    \
    ITEM(NGX_FR_INVALID_PARAMETER)          /* (19) */\
\
    ITEM(NGX_E_AGAIN)                       /* -1 ... */    \
    ITEM(NGX_E_MSG_SIZE)                \
    ITEM(NGX_ECONNECT_FAIL)             \
    ITEM(NGX_EBUFFER_SIZE_NOT_POWER_2)  \
    ITEM(NGX_EALREADY_CLOSED)           \
    ITEM(NGX_ESOCKET_CLOSED)            \
    ITEM(NGX_ESOCKET_WRONG_TYPE)        \
    ITEM(NGX_ESOCKET_NO_DIRECTION)      \
    ITEM(NGX_ECONNECT_FAIL_WRONG_PORT)  \
    ITEM(NGX_ECONNECT_FAIL_WRONG_TYPE)  \
    ITEM(NGX_ECOPY_NEEDED)              \
    ITEM(NGX_EUNSUPPORTED)              \
    ITEM(NGX_EIN_PROXY)                 \
    ITEM(NGX_ENO_DATA_BUFFER)           \
    ITEM(NGX_EALREADY_CONNECTED)        \
    ITEM(NGX_ENOT_CONNECTED)            \
    ITEM(NGX_EOOB)                      \
    ITEM(NGX_EBAD_FLAGS)                \
    ITEM(NGX_EUSER_FULFILL_ERROR)       /* ... -19 */       \
    ITEM(NGX_EOPNOTSUPP)                \
    ITEM(NGX_ENOSPC)                    \
    ITEM(NGX_EXDEV)                     \
    ITEM(NGX_EAGAIN)                    \
    ITEM(NGX_ENOPROTOOPT)               \
    ITEM(NGX_EADDRINUSE)                \
    ITEM(NGX_ECONNRESET)                \
    ITEM(NGX_ENOTCONN)                  \
    ITEM(NGX_EPIPE)                     \
    ITEM(NGX_ETIMEDOUT)                 \
    ITEM(NGX_ECONNREFUSED)              \
    ITEM(NGX_ENETDOWN)                  \
    ITEM(NGX_ENETUNREACH)               \
    ITEM(NGX_EHOSTDOWN)                 \
    ITEM(NGX_EHOSTUNREACH)              \
    ITEM(NGX_EINPROGRESS)               \
    ITEM(NGX_ECONNABORTED)              \
    ITEM(NGX_EINTR)                     \
    ITEM(NGX_ENOTDIR)                   \
    ITEM(NGX_ELOOP)

#define NGX_FILE_ERROR  NGX_ERROR

#define NGX_EEXIST_FILE  NGX_EEXIST
#define NGX_ENOMOREFILES NGX_EEXIST
#define NGX_ENOENT       NGX_EEXIST
#define NGX_ENFILE      NGX_EMFILE
#define NGX_ENOSYS      NGX_EOPNOTSUPP

DECLARE_ENUM(ngx_err_e, NGX_ER_LIST)

u_char * ngx_strerror(ngx_err_t err, u_char *errstr, size_t size);

#define ngx_errno                  errno
#define ngx_socket_errno           errno
#define ngx_set_errno(err)         errno = err
#define ngx_set_socket_errno(err)  errno = err

#endif //CHERIOS_NGX_ERRNO_H
