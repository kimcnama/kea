#ifndef __KEA_STREAM_HELPERS_H
#define __KEA_STREAM_HELPERS_H

#include <stdbool.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)((char *)__mptr - offsetof(type,member));})

bool kea_is_stream_invalid(struct kea_stream *stream);
int kea_stream_write(struct kea_stream *stream, const char *buf, unsigned len);

#endif /* __KEA_STREAM_HELPERS_H */