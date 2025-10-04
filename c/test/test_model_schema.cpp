#include <gtest/gtest.h>

extern "C" {
    #include "kea/kea.h"
    #include "kea/kea_cmds.h"
    #include "kea/kea_model.h"
}

#include "test_util_model_gen.hpp"
#include "test_util_stream.hpp"

class ModelSchemaTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        kea_clear_all_models();
        kea_register_clear_all_streams();

    }
    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }
};

// A simple test case
TEST_F(ModelSchemaTestFixture, get_model_schema_test) {
    
    int i;
    int rc;
    
    TestModelGen model_gen;
    auto &m = model_gen.addModel(8, 16);

    // kea_clear_all_models();
    // kea_register_clear_all_streams();
    TestStreamPair st(128, 256);
    auto serv = st.getServerStream();
    rc = kea_register_stream(serv->getCStream(), serv->getBuf(), serv->getBufSize());
    EXPECT_EQ(0, rc);
    EXPECT_EQ(kea_model_register(m.getCModelHdr(), false), 0);
    
    EXPECT_TRUE(kea_req_get_model_schema(&st.m_client.m_stream, m.getModelId(), true, 1));
    kea_process();

    char rsp_buf[1024];
    #define MAX_ATTRS 64
    struct kea_cmd_model_attr *attrs[MAX_ATTRS];
    int rsp_len = st.getClientRsp(rsp_buf, sizeof(rsp_buf));
    EXPECT_GT(rsp_len, 0);

    int num_objs = kea_cmd_get_model_schema_decode_rsp(rsp_buf, rsp_len, attrs, MAX_ATTRS);
    EXPECT_EQ(num_objs, m.m_hdr.num_static_objs + m.m_hdr.num_dyn_objs);
    for (i = 0; i < num_objs; ++i) {
        printf("Attr %d: obj_id=%d, collection_type=%d, type=%d, size_bytes=%d, name_len=%d, name=%.*s\n",
               i, attrs[i]->obj_id, attrs[i]->collection_type, attrs[i]->type,
               attrs[i]->size_bytes, attrs[i]->name_len,
               attrs[i]->name_len, attrs[i]->name);
    }
}

