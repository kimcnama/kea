#ifndef __KEA_PUBLIC_IF_H
#define __KEA_PUBLIC_IF_H

#ifndef CONFIG_KEA_STREAM_METADATA_SIZE
#define CONFIG_KEA_STREAM_METADATA_SIZE 32
#endif

struct kea_stream {
    char metadata[CONFIG_KEA_STREAM_METADATA_SIZE];
    void *priv_data;
    int (*read)(struct kea_stream*, char *buf, unsigned buf_size);
    int (*write)(struct kea_stream*, const char *buf, unsigned len);
};

int kea_register_stream(struct kea_stream *stream);
void kea_register_clear_all_streams(void);

#ifndef CONFIG_KEA_MAX_NUM_STREAMS
#define CONFIG_KEA_MAX_NUM_STREAMS 4
#endif /* CONFIG_KEA_MAX_NUM_STREAMS */

#ifndef CONFIG_KEA_MAX_REQ_BUF_SIZE
#define CONFIG_KEA_MAX_REQ_BUF_SIZE 256
#endif /* CONFIG_KEA_MAX_REQ_BUF_SIZE */

#define KEA_PRTOCOL_VERSION 0

void kea_process(void);

#endif /* __KEA_PUBLIC_IF_H */