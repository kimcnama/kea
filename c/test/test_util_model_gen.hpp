#pragma once

#include <string>
#include <vector>
#include <utility>

extern "C" {
    #include "kea/kea.h"
    #include "kea/kea_model.h"
}

class TestObjCreator {
public:
    TestObjCreator(
        unsigned obj_id, enum kea_data_collection_types collection_type, enum kea_data_types type, unsigned num_elems);
    ~TestObjCreator() = default;

    struct kea_obj_access_hdr m_prim;
    std::unique_ptr<char[]> m_self;
    char m_obj_name[32];
};

class TestModelCreator {
public:
    TestModelCreator(uint16_t model_id, unsigned num_static_objs, unsigned num_dyn_objs);
    ~TestModelCreator() = default;

    uint16_t getModelId() const { return m_hdr.model_id; }
    std::string getModelName() const { return std::string(m_hdr.name); }
    struct kea_model_hdr *getCModelHdr() { return &m_hdr; }

    static constexpr unsigned max_models = 64;
    struct kea_model_hdr m_hdr;
    char m_model_name[32];
    std::vector<std::unique_ptr<TestObjCreator>> m_objs;

private:
    void createObj(unsigned obj_id);
    unsigned m_cur_obj_id = 0;
    struct kea_obj_access_hdr *m_c_objs[max_models];
};

class TestModelGen {
public:
    TestModelGen() = default;
    ~TestModelGen() = default;

    TestModelCreator &addModel(unsigned num_static_objs, unsigned num_dyn_objs);
    TestModelCreator &getModel(unsigned idx);
    
    std::vector<std::unique_ptr<TestModelCreator>> m_models;
    int m_next_model_id = 1;
};