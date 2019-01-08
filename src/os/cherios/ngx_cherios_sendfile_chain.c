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
#include <ngx_event.h>
#include <sockets.h>


#define SF_DRB_SIZE 1024

struct request_aio {
    uni_dir_socket_requester* for_requester;
    ngx_event_t ev;

};

typedef struct proxy_pair {
    uni_dir_socket_fulfiller ff;
    data_ring_buffer drb;
    char drb_data[SF_DRB_SIZE];
    struct requester_32 req;
} proxy_pair;

void init_pair(proxy_pair* pp) {
    init_data_buffer(&pp->drb,pp->drb_data, SF_DRB_SIZE);
    socket_internal_requester_init(&pp->req.r, 32, SOCK_TYPE_PUSH, &pp->drb);
    socket_internal_fulfiller_init(&pp->ff, SOCK_TYPE_PUSH);
    socket_internal_fulfiller_connect(&pp->ff, socket_internal_make_read_only(&pp->req.r));
    socket_internal_requester_connect(&pp->req.r);
}

proxy_pair* alloc_proxy(ngx_pool_t* pool) {
    proxy_pair* pp = ngx_pnalloc(pool, sizeof(proxy_pair));
    init_pair(pp);
    return pp;
}

proxy_pair* get_pair_for(ngx_pool_t* pool, ngx_file_t* file) {
    if(!file->aio_sendfile_arg) {
        file->aio_sendfile_arg = (void*)alloc_proxy(pool);
    }

    return (proxy_pair*)file->aio_sendfile_arg;
}

ngx_chain_t *
ngx_cherios_sendfile_chain(ngx_connection_t *c, ngx_chain_t *in, off_t limit)
{

    ngx_event_t   *wev;

    wev = c->write;

    if(!wev->ready) {
        return in;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, c->log, 0, "sendfile chain");

    if (limit == 0 || limit > (off_t) (NGX_MAX_SIZE_T_VALUE - ngx_pagesize)) {
        limit = NGX_MAX_SIZE_T_VALUE - ngx_pagesize;
    }

    size_t total = 0;
    size_t col;
    ssize_t res;

    u_char* prev = NULL;
    u_char* base = NULL;

    assert_int_ex(c->fd->con_type & CONNECT_PUSH_WRITE, !=, 0);

    uni_dir_socket_requester* req = c->fd->write.push_writer;

    // Send everything up to the first file
    for ( /* void */ ; in && total < limit; in = in->next) {

        if(ngx_buf_special(in->buf)) continue;

        if(in->buf->in_file) {
            if(base) {
                socket_internal_request_ind(req, base, col, 0);
                //c->buffered |= NGX_LOWLEVEL_BUFFERED;
                base = NULL;
            }

            // TODO for some godforsaken reason we may need to coellesce file write.
            // TODO this doesn't really matter for us as we can enqueue multiple sendfiles

            assert_int_ex(in->buf->file->fd->con_type & CONNECT_PULL_READ, !=, 0);

            uni_dir_socket_requester* read = in->buf->file->fd->read.pull_reader;

            size_t file_size = in->buf->file_last - in->buf->file_pos;

            assert( file_size != 0);

            size_t to_send = file_size > (limit-total) ? limit-total : file_size;

            res = socket_internal_requester_space_wait(read, 1, 1, 0);

            if(res != 0) {assert(0);} // TODO

            res = socket_internal_requester_space_wait(req, 1, 1, 0);

            if(res != 0) {assert(0);} // TODO

            proxy_pair* pp = get_pair_for(c->pool, in->buf->file);

            assert(pp != NULL);

            ngx_log_debug1(NGX_LOG_DEBUG_HTTP, c->log, 0, "Proxy join of %lx bytes\n", to_send);

            res = socket_internal_request_proxy_join(read, &pp->req.r, &pp->drb, to_send, 0, req, &pp->ff, 0);
            //c->buffered |= NGX_LOWLEVEL_BUFFERED;

            assert_int_ex(res, ==, 0);

            in->buf->file_pos += to_send;
            total +=to_send;

            if(to_send != file_size) break;

            // TODO do we need some kind of AIO event in order to close the files at the right time? (wev)
            continue;

        }

        if(!ngx_buf_in_memory(in->buf)) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "bad buf in output chain "
                                  "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          in->buf->temporary,
                          in->buf->recycled,
                          in->buf->in_file,
                          in->buf->start,
                          in->buf->pos,
                          in->buf->last,
                          in->buf->file,
                          in->buf->file_pos,
                          in->buf->file_last);

            ngx_debug_point();

            return NGX_CHAIN_ERROR;
        }

        size_t size = in->buf->last - in->buf->pos;

        size_t to_send = size > limit - total ? limit - total : size;

        if((prev != in->buf->pos)) {
            if(base) {
                socket_internal_request_ind(req, base, col, 0);
                //c->buffered |= NGX_LOWLEVEL_BUFFERED;
            }

            res = socket_internal_requester_space_wait(req, 1, 1, 0);

            if(res != 0) {assert(0);} // TODO

            base = in->buf->pos;
            col = 0;
        }

        col += to_send;
        prev = in->buf->pos + to_send;
        in->buf->pos = prev;
        total +=to_send;

        if(to_send != size) break;

    }

    if(base) {
        socket_internal_request_ind(req, base, col, 0);
        //c->buffered |= NGX_LOWLEVEL_BUFFERED;
    }

    return in;
}