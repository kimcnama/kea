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
TEST(SampleTest, StremRegister) {
    
    int i;
    int rc;

    struct kea_stream stream1 = {
        .metadata = "stream1",
        .priv_data = nullptr,
        .read = nullptr,
        .write = nullptr
    };
    
    rc = kea_register_stream(nullptr);
    EXPECT_EQ(rc, -EINVAL);
    rc = kea_register_stream(&stream1);
    EXPECT_EQ(rc, -EINVAL);
    stream1.read = stream_read;
    rc = kea_register_stream(&stream1);
    EXPECT_EQ(rc, -EINVAL);
    stream1.write = stream_write;

    for (i = 0; i < CONFIG_KEA_MAX_NUM_STREAMS; ++i) {
        rc = kea_register_stream(&stream1);
        EXPECT_EQ(rc, 0);
    }

    rc = kea_register_stream(&stream1);
    EXPECT_EQ(rc, -ENOMEM);
}

struct stream_priv_data {
    unsigned resp_len;
    bool is_proc;
    char buf[CONFIG_KEA_MAX_REQ_BUF_SIZE];
};

static int get_all_models_stream_read(struct kea_stream* stream, char *buf, unsigned buf_size)
{
    printf("Reading from stream (%s)\n", stream->metadata);
    struct stream_priv_data *priv = (struct stream_priv_data *)stream->priv_data;
    if (!priv->is_proc) {
        memcpy(buf, priv->buf, priv->resp_len);
        priv->is_proc = true;
        return priv->resp_len;
    }
    return 0;
}

static int get_all_models_stream_write(struct kea_stream* stream, const char *buf, unsigned len)
{
    printf("Writing to stream (%s): %.*s\n", stream->metadata, len, buf);
    struct stream_priv_data *priv = (struct stream_priv_data *)stream->priv_data;
    priv->resp_len = len;
    priv->is_proc = false;
    memcpy(priv->buf, buf, len);
    return len;
}

int _myint = 0xdeadbeef;
struct kea_obj_access_hdr obj1 = {
    .obj_id = 0,
    .init_status = 1,
    .collection_type = KEA_COLLECTION_TYPE_SCALAR,
    .type = KEA_TYPE_INT32,
    .name = (char *)"myint",
    .next = nullptr,
    .self = &_myint
};
struct kea_obj_access_hdr obj2;

struct kea_obj_access_hdr *static_objs[] = {
    &obj1,
    &obj2,
};

struct kea_model_hdr test_model = {
    .model_id = 1,
    .name = (char *)"test_model",
    .hash = {0},
    .num_static_objs = 2,
    .num_dyn_objs = 0,
    .dyn_objs = nullptr,
    .objs = static_objs
};

TEST(SampleTest, GetAllModels) {
    
    int i;
    int rc;
    struct stream_priv_data priv_data1 = {0};

    struct kea_stream stream1 = {
        .metadata = "get_all_models_stream",
        .priv_data = &priv_data1,
        .read = get_all_models_stream_read,
        .write = get_all_models_stream_write
    };
    
    kea_register_clear_all_streams();
    EXPECT_EQ(kea_register_stream(&stream1), 0);
    EXPECT_TRUE(kea_req_get_all_models(&stream1));
    kea_process();

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}