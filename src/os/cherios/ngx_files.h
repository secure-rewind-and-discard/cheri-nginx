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
#ifndef CHERIOS_NGX_FILES_H
#define CHERIOS_NGX_FILES_H

#include "stdio.h"
#include "unistd.h"
#include "ngx_user.h"

typedef FILE_t ngx_fd_t;
// Should this be cast from an error value?
#define NGX_INVALID_FILE (FILE_t)NULL

typedef uint32_t mode_t;

#if (NGX_HAVE_FILE_AIO)

extern ngx_uint_t  ngx_file_aio;

typedef struct aiocb {
    /* The order of these fields is implementation-dependent */

    ngx_fd_t        aio_fildes;     /* File descriptor */
    off_t           aio_offset;     /* File offset */
    volatile void  *aio_buf;        /* Location of buffer */
    size_t          aio_nbytes;     /* Length of transfer */

    /* Various implementation-internal fields not shown */
} ngx_aiocb_t;

#endif

typedef struct stat {
//    dev_t     st_dev;         /* ID of device containing file */
//    ino_t     st_ino;         /* Inode number */
    mode_t    st_mode;        /* File type and mode */
//    nlink_t   st_nlink;       /* Number of hard links */
    uid_t     st_uid;         /* User ID of owner */
    gid_t     st_gid;         /* Group ID of owner */
//    dev_t     st_rdev;        /* Device ID (if special file) */
    off_t     st_size;        /* Total size, in bytes */
//    blksize_t st_blksize;     /* Block size for filesystem I/O */
//    blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
} ngx_file_info_t;

// TODO.
typedef struct {
    dir_token_t token;

    int valid_info;
    // As we walk through the directory set this

    _unsafe FILINFO info;

} ngx_dir_t;

#define ngx_de_is_dir(dir)                                                   \
    ((dir)->info.fattrib & AM_DIR)
#define ngx_de_is_file(dir)                                                  \
    (!ngx_de_is_dir(dir))
#define ngx_de_is_link(dir)                                                  \
    (0)

#define ngx_de_name(dir)            ((u_char *) (dir)->info.fname)
#define ngx_de_namelen(dir)         strlen(ngx_de_name(dir))
#define ngx_de_size(dir)            (dir)->info.fsize
#define ngx_de_fs_size(dir)         (dir)->info.fsize
#define ngx_de_access(dir)          0
#define ngx_de_mtime(dir)           (dir)->info.ftime

typedef uint64_t ngx_file_uniq_t;

#define ngx_stderr stderr
#define ngx_stdout stdout

ssize_t ngx_read_fd(ngx_fd_t fd, void *buf, size_t size);
#define ngx_read_fd_n               "ReadFileFD()"


ssize_t ngx_write_fd(ngx_fd_t fd, void *buf, size_t size);
#define ngx_write_fd_n              "WriteFileFD()"

ssize_t ngxrecv(FILE_t file,  const void *buf, size_t len, int flags);
ssize_t ngxsend(FILE_t file,  const void *buf, size_t len, int flags);

ssize_t ngx_write_file(ngx_file_t *file, u_char *buf, size_t size,
                       off_t offset);

ssize_t ngx_write_chain_to_file(ngx_file_t *file, ngx_chain_t *ce,
                                off_t offset, ngx_pool_t *pool);


ssize_t readv(FILE_t fd, const struct iovec *vector, int count);
ssize_t writev(FILE_t fd, const struct iovec *vector, int count);

static inline ssize_t sendto(FILE_t sockfd, const void *buf, size_t len, int flags,
                             const struct sockaddr *dest_addr, socklen_t addrlen) {
    return socket_send(sockfd, buf, len, flags);
}

ssize_t sendmsg(FILE_t sockfd, const struct msghdr *msg, int flags);
ssize_t recvmsg(FILE_t sockfd, struct msghdr *msg, int flags);

int aio_read(struct aiocb *aiocbp);
ssize_t aio_return(struct aiocb *aiocbp);
int aio_error(const struct aiocb *aiocbp);

#define ngx_close_file close
#define ngx_close_file_n           "Close()"

#define ngx_delete_file(name)       0
#define ngx_delete_file_n           "Delete()"

ngx_fd_t ngx_open_file(u_char *name, u_long mode, u_long create, u_long access);
#define ngx_open_file_n             "OpenFile()"

ngx_err_t ngx_create_dir(const char * name, u_long access);
#define ngx_create_dir_n         "CreateDir()"

ngx_err_t ngx_set_stderr(ngx_fd_t fd);
#define ngx_set_stderr_n         "SetStderr()"

ngx_err_t ngx_rename_file(const char* o, const char* n);
#define ngx_rename_file_n        "Rename()"

ngx_int_t ngx_file_aio_init(ngx_file_t *file, ngx_pool_t *pool);
ssize_t ngx_file_aio_read(ngx_file_t *file, u_char *buf, size_t size,
                          off_t offset, ngx_pool_t *pool);
ssize_t ngx_read_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset);
#define ngx_read_file_n "Read()"

#define ngx_read_ahead(fd, n)    0
#define ngx_read_ahead_n         "ngx_read_ahead_n"

#define ngx_directio_on(fd)      0
#define ngx_directio_on_n        "ngx_directio_on_n"

ngx_fd_t ngx_open_tempfile(u_char *name, ngx_uint_t persistent,
                           ngx_uint_t access);
#define ngx_open_tempfile_n      "OpenTmp()"

ngx_err_t ngx_file_info(const char* name, ngx_file_info_t* info);
#define ngx_file_info_n "FileInfo()"

ngx_err_t ngx_fd_info(ngx_fd_t fd, ngx_file_info_t* info);
#define ngx_fd_info_n "FDInfo()"

ngx_file_uniq_t ngx_file_uniq(ngx_file_info_t* fi);
time_t ngx_file_mtime(ngx_file_info_t* fi);

#define ngx_file_size(fi) ((fi)->st_size)
#define ngx_file_fs_size(fi) ngx_file_size(fi)
#define ngx_is_dir(fi) (((fi)->st_mode & AM_DIR) != 0)
#define ngx_is_file(fi) (((fi)->st_mode & AM_DIR) == 0)
#define ngx_is_link(fi) 0
#define ngx_is_exec(fi) 0
#define ngx_file_uniq(fi) 0 // TODO 0 is not very unique
#define ngx_file_mtime(fi) 0

ngx_int_t ngx_open_dir(ngx_str_t *name, ngx_dir_t *dir);
#define ngx_open_dir_n "OpenDir()"

ngx_int_t ngx_read_dir(ngx_dir_t *dir);
#define ngx_read_dir_n           "readdir()"


static ngx_inline ngx_int_t ngx_de_info(u_char *name, ngx_dir_t *dir) {
    return 0;
}

#define ngx_de_info_n "DirInfo()"


ngx_err_t ngx_close_dir(ngx_dir_t* dir);
#define ngx_close_dir_n "CloseDir"


#define NGX_DIR_MASK_LEN         0

ngx_int_t ngx_set_file_time(u_char *name, ngx_fd_t fd, time_t s);
#define ngx_set_file_time_n      "utimes()"

int chown(const char *pathname,
              uid_t owner,
              gid_t group);
int chmod(const char *pathname, mode_t mode);

#define ngx_dir_access(a)           (a)

#define ngx_write_console        ngx_write_fd

#define ngx_linefeed(p)          *p++ = LF;
#define NGX_LINEFEED_SIZE        1
#define NGX_LINEFEED             "\x0a"
#define ngx_path_separator(c)    ((c) == '/')

// TODO

#define NGX_FILE_APPEND             ((1 << 16) | FA_WRITE)
#define NGX_FILE_CREATE_OR_OPEN     FA_OPEN_ALWAYS
#define NGX_FILE_DEFAULT_ACCESS     0000
#define NGX_FILE_OWNER_ACCESS       0000
#define NGX_FILE_RDWR               (FA_READ | FA_WRITE)
#define NGX_FILE_RDONLY             FA_READ
#define NGX_FILE_OPEN               FA_OPEN_EXISTING
#define NGX_FILE_WRONLY             FA_WRITE
#define NGX_FILE_NONBLOCK           (1 << 17)
#define NGX_FILE_TRUNCATE           ((1 << 18))

typedef struct {
    size_t   gl_pathc;    /* Count of paths matched so far  */
    char   **gl_pathv;    /* List of matched pathnames.  */
    size_t   gl_offs;     /* Slots to reserve in gl_pathv.  */
} glob_t;

typedef struct {
    size_t                       n;
    glob_t                       pglob;
    u_char                      *pattern;
    ngx_log_t                   *log;
    ngx_uint_t                   test;
} ngx_glob_t;

ngx_err_t ngx_trylock_fd(ngx_fd_t fd);
ngx_err_t ngx_lock_fd(ngx_fd_t fd);
ngx_err_t ngx_unlock_fd(ngx_fd_t fd);

#define ngx_trylock_fd_n "TRYLOCK_FD()"
#define ngx_lock_fd_n "LOCK_FD()"
#define ngx_unlock_fd_n "UNLOCK_FD()"

ngx_int_t ngx_open_glob(ngx_glob_t *gl);
#define ngx_open_glob_n          "glob()"
ngx_int_t ngx_read_glob(ngx_glob_t *gl, ngx_str_t *name);
void ngx_close_glob(ngx_glob_t *gl);

#define S_ISUID     04000
#define S_ISGID     02000
#define S_ISVTX     01000
#define S_IRWXU     00700
#define S_IRUSR     00400
#define S_IWUSR     00200
#define S_IXUSR     00100
#define S_IRWXG     00070
#define S_IRGRP     00040
#define S_IWGRP     00020
#define S_IXGRP     00010
#define S_IRWXO     00007
#define S_IROTH     00004
#define S_IWOTH     00002
#define S_IXOTH     00001

#define ngx_realpath(in, out) (in)
#define ngx_realpath_n              ""

size_t ngx_fs_bsize(u_char *name);

#endif //CHERIOS_NGX_FILES_H
