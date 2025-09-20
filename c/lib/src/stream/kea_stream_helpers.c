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
        rc = stream->write(stream->metadata, buf, len);
        
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