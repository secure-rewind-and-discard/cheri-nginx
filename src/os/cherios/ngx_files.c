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
#include <ff.h>
#include <ngx_string.h>
#include <sockets.h>
#include <ngx_event.h>
#include "ngx_files.h"

static int map_fs_errors(FRESULT fresult) {
    if(fresult != FR_OK) {
        ngx_errno = (int)fresult; // Organised to match
        return NGX_ERROR;
    }
    return NGX_OK;
}

static ssize_t map_sock_errors(ssize_t sock_er) {
    if(sock_er < 0) {
        // TODO map some sock errors
        if(sock_er == E_AGAIN) return NGX_AGAIN;
        ngx_errno = (int)((-sock_er) + (NGX_E_AGAIN-1));
        return NGX_ERROR;
    }
    return sock_er;
}

ssize_t ngx_read_fd(ngx_fd_t fd, void *buf, size_t size) {
    return map_sock_errors(read(fd, buf, size));
}

ssize_t ngx_write_fd(ngx_fd_t fd, void *buf, size_t size) {
    return map_sock_errors(write(fd, buf, size));
}

ssize_t ngxrecv(FILE_t file,  const void *buf, size_t len, int flags) {
    return map_sock_errors(socket_recv(file,buf,len,flags));
}

ssize_t ngxsend(FILE_t file,  const void *buf, size_t len, int flags) {
    return map_sock_errors(socket_send(file,buf,len,flags));
}

ssize_t ngx_write_chain_to_file(ngx_file_t *file, ngx_chain_t *ce,
                                off_t offset, ngx_pool_t *pool) {
    ssize_t res = lseek(file->fd, offset, SEEK_SET);
    if(res < 0) return map_sock_errors(res);

    size_t total = 0;

    while(ce) {
        u_char * buf = ce->buf->pos;
        size_t len = 0;

#define SZ(C) ((C)->buf->last - (C)->buf->pos)
        while(ce && ((size_t)buf + len == (size_t)ce->buf->pos) && cheri_getlen(buf) >= (len + SZ(ce))) {
            len += SZ(ce);
            ce = ce->next;
        }

        assert(len != 0);

        res = write(file->fd, buf, len);

        if(res < 0) return total > 0 ? (ssize_t)total : map_sock_errors(res);

        total += len;
    }

    return total;
}

ssize_t
ngx_read_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset)
{
    ssize_t  n;

    ngx_log_debug4(NGX_LOG_DEBUG_CORE, file->log, 0,
                   "read: %d, %p, %uz, %O", file->fd->sockn, buf, size, offset);


    if (file->sys_offset != offset) {
        if (lseek(file->fd, offset, SEEK_SET) != 0) {
            ngx_log_error(NGX_LOG_CRIT, file->log, ngx_errno,
                          "lseek() \"%s\" failed", file->name.data);
            return NGX_ERROR;
        }

        file->sys_offset = offset;
    }

    n = read(file->fd, buf, size);

    if (n < 0) {
        map_sock_errors(n);
        ngx_log_error(NGX_LOG_CRIT, file->log, ngx_errno,
                      "read() \"%s\" failed", file->name.data);
        return NGX_ERROR;
    }

    file->sys_offset += n;

    file->offset += n;

    return n;
}


ssize_t
ngx_write_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset)
{
    ssize_t    n, written;
    ngx_err_t  err;

    ngx_log_debug4(NGX_LOG_DEBUG_CORE, file->log, 0,
                   "write: %d, %p, %uz, %O", file->fd, buf, size, offset);

    written = 0;

    if (file->sys_offset != offset) {
        if (lseek(file->fd, offset, SEEK_SET) != 0) {
            ngx_log_error(NGX_LOG_CRIT, file->log, ngx_errno,
                          "lseek() \"%s\" failed", file->name.data);
            return NGX_ERROR;
        }

        file->sys_offset = offset;
    }

    for ( ;; ) {
        n = write(file->fd, buf + written, size);

        if (n < 0) {
            map_sock_errors(n);

            err = ngx_errno;

            if (err == NGX_EINTR) {
                ngx_log_debug0(NGX_LOG_DEBUG_CORE, file->log, err,
                               "write() was interrupted");
                continue;
            }

            ngx_log_error(NGX_LOG_CRIT, file->log, err,
                          "write() \"%s\" failed", file->name.data);
            return NGX_ERROR;
        }

        file->sys_offset += n;
        file->offset += n;
        written += n;

        if ((size_t) n == size) {
            return written;
        }

        size -= n;
    }
}

#if NGX_HAVE_FILE_AIO
int aio_read(struct aiocb *aiocbp) {

    assert(aiocbp->aio_fildes->con_type & CONNECT_PULL_READ);

    // TODO seek to offset
    uni_dir_socket_requester* req = aiocbp->aio_fildes->read.pull_reader;

    ssize_t res = socket_internal_requester_space_wait(req,1,1,0);

    if(res != 0) return (int)map_sock_errors(res);

    res = socket_internal_request_ind(req,aiocbp->aio_buf,aiocbp->aio_nbytes,0);

    if(res == 0) {
        aiocbp->aio_fildes->flags |= (SOCKF_POLL_READ_MEANS_EMPTY);
    }

    return (int)map_sock_errors(res);
}

ssize_t aio_return(struct aiocb *aiocbp) {
    uni_dir_socket_requester* req = aiocbp->aio_fildes->read.pull_reader;

    ssize_t res = socket_internal_requester_wait_all_finish(req, 1);

    if(res == 0)  {
        res = aiocbp->aio_nbytes;
        aiocbp->aio_fildes->flags &= ~(SOCKF_POLL_READ_MEANS_EMPTY);
    }

    return map_sock_errors(res);
}

int aio_error(const struct aiocb *aiocbp) {
    uni_dir_socket_requester* req = aiocbp->aio_fildes->read.pull_reader;

    ssize_t res = socket_internal_requester_wait_all_finish(req, 1);

    if(res == E_AGAIN) return NGX_EINPROGRESS;

    return (int)map_sock_errors(res);
}
#endif

ngx_err_t ngx_rename_file(const char* o, const char* n) {
    return map_fs_errors(rename(o, n));
}

ngx_fd_t ngx_open_file(u_char *name, u_long mode, u_long create, u_long access) {

    mode |= create;

    u_long nonblock = mode & NGX_FILE_NONBLOCK;

    ERROR_T(FILE_t) result = open_er(name, (int)(mode & 0xFF), (nonblock ? MSG_DONT_WAIT : MSG_NONE) | SOCKF_GIVE_SOCK_N, NULL, NULL);

    if(IS_VALID(result)) {
        ssize_t res = 0;
        if(mode & NGX_FILE_TRUNCATE) {
            res = truncate(result.val);
        }
        if(res < 0) {
            map_sock_errors(res);
            return NULL;
        }
        if((mode & NGX_FILE_APPEND) == NGX_FILE_APPEND) {
            res = lseek(result.val, 0, SEEK_END);
        }
        if(res < 0) {
            map_sock_errors(res);
            return NULL;
        }
    } else {
        if((ssize_t)result.er < 0) map_sock_errors((ssize_t)result.er);
        else map_fs_errors((FRESULT)result.er);
        return NULL;
    }

    return result.val;
}

ngx_fd_t ngx_open_tempfile(u_char *name, ngx_uint_t persistent,
                           ngx_uint_t access) {
    static int tmp_ctr = 0;
    char tmp_name[] = "tmpXX";

    assert(tmp_ctr <= 0xFF);

    itoa(tmp_ctr, tmp_name+3,16);

    return ngx_open_file((u_char *)tmp_name, 0, 1, access);
}

ngx_err_t ngx_set_stderr(ngx_fd_t fd) {
    // stderr = fd; For now don't allow redirect TODO change back
    //return NGX_FILE_ERROR;
    return NGX_OK;
}

ngx_err_t ngx_create_dir(const char * name, u_long access) {
    return map_fs_errors(mkdir(name));
}

ngx_err_t ngx_file_info(const char* name, ngx_file_info_t* info) {
    _unsafe FILINFO filinfo;

    FRESULT res = stat(name, &filinfo);

    if(res != FR_OK) return map_fs_errors(res);

    info->st_size = filinfo.fsize;
    info->st_gid = NO_GROUP_ID;
    info->st_uid = NO_USER_ID;
    info->st_mode = filinfo.fattrib;
    return NGX_OK;
}

ngx_err_t ngx_fd_info(ngx_fd_t fd, ngx_file_info_t* info) {
    ssize_t fs = filesize(fd);

    if(fs < 0) return map_sock_errors(fs);

    info->st_size = (size_t)fs;
    info->st_gid = NO_GROUP_ID;
    info->st_uid = NO_USER_ID;
    info->st_mode = 0; // TODO infer mode from socket flags

    return NGX_OK;
}

ngx_int_t ngx_set_file_time(u_char *name, ngx_fd_t fd, time_t s) {
    // TODO
    return NGX_OK;
}

ngx_int_t ngx_open_dir(ngx_str_t *name, ngx_dir_t *dir) {
    dir->token = opendir(name->data);
    dir->valid_info = 0;
    return dir->token == NULL ? NGX_ERROR : NGX_OK;
}

ngx_int_t ngx_read_dir(ngx_dir_t *dir) {
    FRESULT res = readdir(dir->token, &dir->info);
    return map_fs_errors(res);
}

ngx_err_t ngx_close_dir(ngx_dir_t* dir) {
    FRESULT res = closedir(dir->token);
    return map_fs_errors(res);
}

int getsockopt(ngx_socket_t sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    // TODO
    assert(0);
    return -1;
}

int setsockopt(ngx_socket_t sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    switch(level) {
        case SOL_SOCKET:
            switch(optname) {
                case SO_REUSEADDR:
                    return 0; // TODO
                case SO_REUSEPORT:
                case SO_RCVBUF:
                case SO_SNDBUF:
                case SO_KEEPALIVE:
                default:{}
            }
            break;
        case IPPROTO_TCP:
            switch(optname) {
                case TCP_NODELAY:
                    return 0; // TODO (probably quite important)
                case TCP_KEEPALIVE:
                case TCP_KEEPIDLE:
                case TCP_KEEPCNT:

                default:{}
            }
            break;
        default:{}
            // unrecognised level
    }
    printf("Socket level %d. optname %d.\n", level, optname);
    sleep(MS_TO_CLOCK(100));
    assert(0);
    return -1;
}

int getsockname(ngx_socket_t sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    // TODO
    assert(0);
    return -1;
}

int ngx_close_socket(ngx_socket_t s) {
    requester_t req =  s->write.push_writer;

    return (int)map_sock_errors(close(s));
}

ngx_err_t ngx_nonblocking(ngx_socket_t s) {
    s->flags |= MSG_DONT_WAIT;
    return NGX_OK;
}

ngx_err_t ngx_blocking(ngx_socket_t s) {
    s->flags &= ~MSG_DONT_WAIT;
    return NGX_OK;
}

int ngx_shutdown_socket(ngx_socket_t s, int how) {
    return (int)map_sock_errors(shutdown(s, how));
}

size_t ngx_sock_to_int(ngx_socket_t sockfd) {
    assert(sockfd->flags & SOCKF_GIVE_SOCK_N);
    return sockfd->sockn;
}

#define GLOB_NOMATCH -1

static int glob(const char *pattern, int flags,
         int (*errfunc) (const char *epath, int eerrno),
         glob_t *pglob) {
    printf(KRED"Tried to glob pattern %s\n"KRST, pattern);
    assert(0);
    while(1);
}

static void globfree(glob_t *pglob) {
    return;
}

ngx_int_t
ngx_open_glob(ngx_glob_t *gl)
{
    int  n;

    n = glob((char *) gl->pattern, 0, NULL, &gl->pglob);

    if (n == 0) {
        return NGX_OK;
    }

#ifdef GLOB_NOMATCH

    if (n == GLOB_NOMATCH && gl->test) {
        return NGX_OK;
    }

#endif

    return NGX_ERROR;
}


ngx_int_t
ngx_read_glob(ngx_glob_t *gl, ngx_str_t *name)
{
    size_t  count;

#ifdef GLOB_NOMATCH
    count = (size_t) gl->pglob.gl_pathc;
#else
    count = (size_t) gl->pglob.gl_matchc;
#endif

    if (gl->n < count) {

        name->len = (size_t) ngx_strlen(gl->pglob.gl_pathv[gl->n]);
        name->data = (u_char *) gl->pglob.gl_pathv[gl->n];
        gl->n++;

        return NGX_OK;
    }

    return NGX_DONE;
}


void
ngx_close_glob(ngx_glob_t *gl)
{
    globfree(&gl->pglob);
}

size_t
ngx_fs_bsize(u_char *name)
{
    return 512;
}

int ngx_tcp_push(ngx_socket_t s) {
    // TODO
    assert(0);
    return 0;
}

ngx_socket_t ngx_socket(int domain, int type, int protocol) {
    ERROR_T(unix_net_sock_ptr) res = socket_or_er(domain, type, protocol);
    if(!IS_VALID(res)) {
        map_sock_errors((ssize_t)res.er);
        return (ngx_socket_t) -1;
    }
    assign_socket_n(&res.val->sock);
    return (ngx_socket_t)res.val;
}

ssize_t readv(FILE_t fd, const struct iovec *vector, int count) {
    ssize_t total = 0;
    for(size_t i = 0; i != count; i++) {
        ssize_t res = socket_recv(fd,vector[i].iov_base,vector[i].iov_len,MSG_NONE);
        if(res < 0) {
            if(total == 0) return res;
            else break;
        }
        total += res;
        if(res != vector[i].iov_len) break;
    }
    return map_sock_errors(total);
}

ssize_t writev(FILE_t fd, const struct iovec *vector, int count) {
    ssize_t total = 0;
    for(size_t i = 0; i != count; i++) {
        ssize_t res = socket_send(fd,vector[i].iov_base,vector[i].iov_len,MSG_NONE);
        if(res < 0) {
            if(total == 0) total = res;
            break;
        }
        total += res;
        if(res != vector[i].iov_len) break;
    }
    return map_sock_errors(total);
}

ssize_t sendmsg(FILE_t sockfd, const struct msghdr *msg, int flags) {
    ssize_t total = 0;
    int count = msg->msg_iovlen;
    const struct iovec *vector = msg->msg_iov;

    for(size_t i = 0; i != count; i++) {
        ssize_t res = socket_send(sockfd,vector[i].iov_base,vector[i].iov_len, flags);
        if(res < 0) {
            if(total == 0) total = res;
            break;
        }
        total += res;
        if(res != vector[i].iov_len) break;
    }
    return map_sock_errors(total);
}

ssize_t recvmsg(FILE_t sockfd, struct msghdr *msg, int flags) {
    ssize_t total = 0;
    int count = msg->msg_iovlen;
    const struct iovec *vector = msg->msg_iov;

    for(size_t i = 0; i != count; i++) {
        ssize_t res = socket_recv(sockfd,vector[i].iov_base,vector[i].iov_len, flags);
        if(res < 0) {
            if(total == 0) total = res;
            break;
        }
        total += res;
        if(res != vector[i].iov_len) break;
    }
    return map_sock_errors(total);
}
