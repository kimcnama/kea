#include <errno.h>
#include "kea/kea.h"
#include "kea_stream_helpers.h"

bool kea_is_stream_invalid(struct kea_stream *stream)
{
    return !stream || !stream->write || !stream->read;
}

int kea_stream_write(struct kea_stream *stream, const char *buf, unsigned len)
{
    int rc;
    int written = 0;
    int retries = 3;

    if (!stream || !stream->write || !buf || len == 0)
        return -EINVAL;

    while (retries-- > 0 && len > 0) {
        rc = stream->write(stream, buf, len);
        
        if (rc <= 0)
            continue;
        
        written += rc;
        len -= (rc <= len ? rc : len);
        buf += rc;
    }

    if (written <= 0 && retries <= 0)
        return -EBUSY;

    return written;
}

void kea_stream_init_rsp_buf(struct kea_stream *stream, unsigned char op_code, unsigned char transaction_id)
{
    struct kea_rsp_hdr *hdr = (struct kea_rsp_hdr*)stream->stream_buf;

    memset(stream->stream_buf, 0x00, stream->stream_buf_size);

    hdr->version = KEA_PRTOCOL_VERSION;
    hdr->op_code = op_code;
    hdr->transaction_id = transaction_id;
    hdr->status = KEA_CMD_STATUS_OK;
    hdr->seq_num = 0;
    hdr->len = 0;
    
    stream->cur_buf_offset = sizeof(struct kea_rsp_hdr);
}

bool kea_stream_send_err_rsp(struct kea_stream *stream, enum kea_cmd_status err_code, unsigned char op_code, unsigned char transaction_id)
{
    struct kea_rsp_hdr *hdr = (struct kea_rsp_hdr*)stream->stream_buf;
    kea_stream_init_rsp_buf(stream, op_code, transaction_id);
    hdr->status = err_code;
    return sizeof(struct kea_rsp_hdr) == kea_stream_write(stream, stream->stream_buf, stream->cur_buf_offset);
}

bool kea_stream_send_initd_err_rsp(struct kea_stream *stream, enum kea_cmd_status err_code)
{
    struct kea_rsp_hdr *hdr = (struct kea_rsp_hdr*)stream->stream_buf;
    hdr->status = err_code;
    hdr->len = 0;
    return sizeof(struct kea_rsp_hdr) == kea_stream_write(stream, stream->stream_buf, stream->cur_buf_offset);
}

bool kea_stream_write_rsp_data(struct kea_stream *stream, const void *data, int data_len)
{
    int copied_bytes, bytes_written;
    int free_space = (int)stream->stream_buf_size - (int)stream->cur_buf_offset;
    char *src_ptr = (char *)data;
    struct kea_rsp_hdr *hdr = stream->stream_buf;

    while (data_len > 0 || free_space == 0) {

        if (free_space > 0) {
            copied_bytes = data_len < free_space ? data_len : free_space;
            memcpy(&stream->stream_buf[stream->cur_buf_offset], src_ptr, copied_bytes);
            stream->cur_buf_offset += copied_bytes;
            data_len -= copied_bytes;
            src_ptr += copied_bytes;
            free_space -= copied_bytes;
            hdr->len += copied_bytes;
        }

        if (free_space == 0) {
            bytes_written = kea_stream_write(stream, stream->stream_buf, stream->cur_buf_offset);
            if (bytes_written < stream->cur_buf_offset) {
                // Failed to write response
                return false;
            }
            
            stream->cur_buf_offset = sizeof(struct kea_rsp_hdr);
            free_space = (int)stream->stream_buf_size - (int)stream->cur_buf_offset;
            hdr->seq_num++;
            hdr->len = 0;
        }

    }

    return true;
}

bool kea_stream_send_rsp_buf_if_nonempty(struct kea_stream *stream)
{
    int remaining_len = stream->cur_buf_offset - sizeof(struct kea_rsp_hdr);
    if (remaining_len > 0) {
        return remaining_len == kea_stream_write(stream, stream->stream_buf, stream->cur_buf_offset);
    }
    return true;
}

bool kea_stream_write_or_err(struct kea_stream *stream, const void *data, int data_len)
{
    if (!kea_stream_write_rsp_data(stream, data, data_len)) {
        (void)kea_stream_send_initd_err_rsp(stream, KEA_CMD_STATUS_STREAM_ERR);
        return false;
    }
    return true;
}