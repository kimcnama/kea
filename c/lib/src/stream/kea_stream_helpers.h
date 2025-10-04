#ifndef __KEA_STREAM_HELPERS_H
#define __KEA_STREAM_HELPERS_H

#include <stdbool.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)((char *)__mptr - offsetof(type,member));})

bool kea_is_stream_invalid(struct kea_stream *stream);
int kea_stream_write(struct kea_stream *stream, const char *buf, unsigned len);

void kea_stream_init_rsp_buf(struct kea_stream *stream, unsigned char op_code, unsigned char transaction_id);
bool kea_stream_send_initd_err_rsp(struct kea_stream *stream, enum kea_cmd_status err_code);
bool kea_stream_send_err_rsp(struct kea_stream *stream, enum kea_cmd_status err_code, unsigned char op_code, unsigned char transaction_id);
bool kea_stream_write_rsp_data(struct kea_stream *stream, const void *data, int data_len);
bool kea_stream_send_rsp_buf_if_nonempty(struct kea_stream *stream);
bool kea_stream_write_or_err(struct kea_stream *stream, const void *data, int data_len);

#endif /* __KEA_STREAM_HELPERS_H */