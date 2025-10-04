#include <gtest/gtest.h>

extern "C" {
    #include "kea/kea.h"
    #include "kea/kea_cmds.h"
    #include "kea/kea_model.h"
}

static int stream_read(struct kea_stream* stream, char *buf, unsigned buf_size)
{
    printf("Reading from stream (%s)\n", stream->metadata);
    memcpy(buf, stream->metadata, strlen(stream->metadata));
    return strlen(stream->metadata);
}

static int stream_write(struct kea_stream* stream, const char *buf, unsigned len)
{
    printf("Writing to stream (%s): %.*s\n", stream->metadata, len, buf);
    return len;
}

// A simple test case
TEST(StreamTests, stream_register) {
    
    int i;
    int rc;

    struct kea_stream stream1 = {
        .metadata = "stream1",
        .priv_data = nullptr,
        .read = nullptr,
        .write = nullptr
    };
    
    kea_register_clear_all_streams();
    rc = kea_register_stream(nullptr, nullptr, 0);
    EXPECT_EQ(rc, -EINVAL);
    rc = kea_register_stream(&stream1, nullptr, 0);
    EXPECT_EQ(rc, -EINVAL);
    stream1.read = stream_read;
    rc = kea_register_stream(&stream1, nullptr, 0);
    EXPECT_EQ(rc, -EINVAL);
    stream1.write = stream_write;

    for (i = 0; i < CONFIG_KEA_MAX_NUM_STREAMS; ++i) {
        rc = kea_register_stream(&stream1, nullptr, 0);
        EXPECT_EQ(rc, 0);
    }

    rc = kea_register_stream(&stream1, nullptr, 0);
    EXPECT_EQ(rc, -ENOMEM);
}

