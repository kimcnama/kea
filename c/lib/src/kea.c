
#include <errno.h>

#include "kea/kea.h"

struct kea_streams {
    unsigned num_registered;
    struct kea_stream streams[CONFIG_KEA_MAX_NUM_STREAMS];
} reg_streams = {0};

static char default_rsp_buf[CONFIG_KEA_DEFAULT_RSP_BUF_SIZE];

int kea_register_stream(struct kea_stream *stream, void *custom_buf, unsigned custom_buf_len)
{
    if (!stream)
        return -EINVAL;

    if (!stream->read || !stream->write)
        return -EINVAL;

    if (custom_buf_len > KEA_MAX_RESP_PKT_LEN + sizeof(struct kea_rsp_hdr))
        return -EINVAL;

    if (reg_streams.num_registered < CONFIG_KEA_MAX_NUM_STREAMS) {

        if (custom_buf && custom_buf_len > 0) {
            stream->stream_buf = (char *)custom_buf;
            stream->stream_buf_size = custom_buf_len;
        } else {
            stream->stream_buf = default_rsp_buf;
            stream->stream_buf_size = sizeof(default_rsp_buf);
        }

        stream->cur_buf_offset = 0;

        memcpy(&reg_streams.streams[reg_streams.num_registered++], stream, sizeof(struct kea_stream));
        return 0;
    } else {
        return -ENOMEM;
    }
}

void kea_register_clear_all_streams(void)
{
    reg_streams.num_registered = 0;
}

void kea_process(void)
{
    unsigned i;
    int read_len;
    char req_buf[CONFIG_KEA_MAX_REQ_BUF_SIZE];

    for (i = 0; i < reg_streams.num_registered; ++i) {
        read_len = reg_streams.streams[i].read(&reg_streams.streams[i], req_buf, CONFIG_KEA_MAX_REQ_BUF_SIZE);
        if (read_len > 0)
            kea_cmd_processor(&reg_streams.streams[i], req_buf, (unsigned)read_len);
    }
}