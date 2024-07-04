#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_http_parser_wrap.h"


int start_routine = 0;
int data_domain_flag = 0;

ngx_http_request_t  *r_copy;
ngx_buf_t           *header_copy;
ngx_pool_t          *pool_copy; 

void *
__ngx_memalign(int udi, size_t alignment, size_t size, ngx_log_t *log)
{
    void  *p;
    int    err;

    err = sdrad_memalign(udi, &p, alignment, size);

    if (err) {
        ngx_log_error(NGX_LOG_EMERG, log, err,
                      "posix_memalign(%uz, %uz) failed", alignment, size);
        p = NULL;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_ALLOC, log, 0,
                   "posix_memalign: %p:%uz @%uz", p, size, alignment);

    return p;
}


ngx_pool_t *
__ngx_create_pool(int udi, size_t size, ngx_log_t *log)
{
    ngx_pool_t  *p;

    p = __ngx_memalign(udi, NGX_POOL_ALIGNMENT, size, log);
    if (p == NULL) {
        return NULL;
    }

    p->d.last = (u_char *) p + sizeof(ngx_pool_t);
    p->d.end = (u_char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_t);
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->chain = NULL;
    p->large = NULL;
    p->cleanup = NULL;
    p->log = log;

    return p;
}



ngx_int_t __real_ngx_http_parse_header_line(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_uint_t allow_underscores); 
ngx_int_t __wrap_ngx_http_parse_header_line(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_uint_t allow_underscores)
{
    ngx_int_t rc;


    cheri_domain_enter(NGX_NESTED_DOMAIN);
    rc = __real_ngx_http_parse_header_line(r,b,allow_underscores);
    cheri_domain_exit();

    return rc;
}


ngx_int_t __real_ngx_http_parse_request_line(ngx_http_request_t *r, ngx_buf_t *b);
ngx_int_t __wrap_ngx_http_parse_request_line(ngx_http_request_t *r, ngx_buf_t *b)
{
    ngx_int_t rc;

    cheri_domain_enter(NGX_NESTED_DOMAIN);
    rc = __real_ngx_http_parse_request_line(r, b);
    cheri_domain_exit();

    return rc;
}


void
__ngx_destroy_pool(ngx_pool_t *pool)
{
    ngx_pool_t          *p, *n;
    ngx_pool_large_t    *l;
    ngx_pool_cleanup_t  *c;

    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                           "run cleanup: %p", c);
            c->handler(c->data);
        }
    }

#if (NGX_DEBUG)

    /*
     * we could allocate the pool->log from this pool
     * so we cannot use this log while free()ing the pool
     */

    for (l = pool->large; l; l = l->next) {
        ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0, "free: %p", l->alloc);
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                       "free: %p, unused: %uz", p, p->d.end - p->d.last);

        if (n == NULL) {
            break;
        }
    }

#endif

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            sdrad_free(NGX_NESTED_DOMAIN, l->alloc);
        }
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        sdrad_free(NGX_NESTED_DOMAIN, p);

        if (n == NULL) {
            break;
        }
    }
}