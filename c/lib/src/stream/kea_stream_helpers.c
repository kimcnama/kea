#include "kea/kea.h"
#include "kea_stream_helpers.h"

bool kea_is_stream_invalid(struct kea_stream *stream)
{
    return !stream || !stream->write || !stream->read;
}

bool kea_stream_write(struct kea_stream *stream, const char *buf, unsigned len)
{
    int written;
    int retries = 3;
    
    if (!stream || !stream->write || !buf || len == 0)
        return false;

    while (retries-- > 0 && len > 0) {
        written = stream->write(stream->metadata, buf, len);
        
        if (written <= 0)
            continue;
        
        written = MIN(written, len);
        len -= written;
        buf += written;
    }
    
    return len == 0;
}