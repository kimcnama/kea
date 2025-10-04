#include <gtest/gtest.h>

extern "C" {
    #include "kea/kea.h"
    #include "kea/kea_cmds.h"
    #include "kea/kea_model.h"
}

#include "test_util_model_gen.hpp"

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
    .name = "myint",
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
    .name = "test_model",
    .hash = {0},
    .num_static_objs = 2,
    .num_dyn_objs = 0,
    .dyn_objs = nullptr,
    .objs = static_objs
};


TEST(GetAllModels, test_get_all_models) {
    
    int i ,rc;
    struct stream_priv_data priv_data1 = {0};

    struct kea_stream stream1 = {
        .metadata = "get_all_models_stream",
        .priv_data = &priv_data1,
        .read = get_all_models_stream_read,
        .write = get_all_models_stream_write
    };
    
    constexpr unsigned num_expected_models = 3;
    TestModelGen model_gen;
    for (i = 1; i <= num_expected_models; ++i) {
        auto &m = model_gen.addModel(i * i, i * i);
        rc = kea_model_register(&m.m_hdr, false);
        EXPECT_EQ(rc, 0);
    }

    kea_register_clear_all_streams();
    EXPECT_EQ(kea_register_stream(&stream1, nullptr, 0), 0);
    EXPECT_TRUE(kea_req_get_all_models(&stream1, true, 1));
    kea_process();

    struct kea_rsp_model_meta *models_info[16];
    unsigned num_models = kea_cmd_get_all_models_decode_rsp(
        priv_data1.buf, priv_data1.resp_len, models_info, 16);
    
    for (i = 0; i < num_models; ++i) {
        printf("Model %d: ID=%d, Name=%.*s\n", i, models_info[i]->model_id,
               models_info[i]->name_len, models_info[i]->name);
        
        EXPECT_EQ(models_info[i]->model_id, model_gen.getModel(i).getModelId());
        EXPECT_EQ(models_info[i]->name_len, model_gen.getModel(i).getModelName().size());
        EXPECT_EQ(strncmp(models_info[i]->name, model_gen.getModel(i).getModelName().c_str(), models_info[i]->name_len), 0);
    }
    
    EXPECT_EQ(num_models, num_expected_models);
    
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}