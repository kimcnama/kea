#include <iostream>
#include <string.h>
#include <memory>

#include "test_util_model_gen.hpp"


TestObjCreator::TestObjCreator(
    unsigned obj_id, enum kea_data_collection_types collection_type, enum kea_data_types type, unsigned num_elems) {

    m_prim.obj_id = obj_id;
    m_prim.init_status = 1;
    m_prim.collection_type = collection_type;
    m_prim.type = type;
    std::string name = "obj_" + std::to_string(obj_id);
    memset(m_obj_name, 0, sizeof(m_obj_name));
    strncpy(m_obj_name, name.c_str(), sizeof(m_obj_name) - 1);
    m_prim.name = m_obj_name;
    m_prim.next = nullptr;
    m_prim.self = nullptr; // Placeholder

    if (collection_type == KEA_COLLECTION_TYPE_SCALAR) {
        m_self = std::make_unique<char[]>(sizeof(long long));
        m_prim.self = m_self.get(); // Just be lazy to begin with

        // Fill m_prim.self with random data
        int sz = kea_data_type_sizes[type];
        for (int i = 0; i < sz; i += sizeof(int)) {
            int random_value = rand();
            memcpy(&((char*)m_prim.self)[i], &random_value, std::min(sz, sz - i));
        }
    }
}


TestModelCreator::TestModelCreator(uint16_t model_id, unsigned num_static_objs, unsigned num_dyn_objs) {
    m_hdr.model_id = model_id;

    std::string name = "test_model_" + std::to_string(model_id);
    memset(m_model_name, 0, sizeof(m_model_name));
    strncpy(m_model_name, name.c_str(), sizeof(m_model_name) - 1);
    m_hdr.name = m_model_name;
    
    for (int i = 0; i < MODEL_HASH_SZ; i += sizeof(int)) {
        int rand_val = rand();
        memcpy(&m_hdr.hash[i], &rand_val, std::min((unsigned)(MODEL_HASH_SZ - i), (unsigned)sizeof(int)));
    }
    
    
    m_hdr.num_static_objs = num_static_objs;
    m_hdr.num_dyn_objs = num_dyn_objs;

    memset(m_c_objs, 0, sizeof(m_c_objs));
    m_hdr.dyn_objs = NULL;
    m_hdr.objs = m_c_objs;

    unsigned total_objs = num_static_objs + num_dyn_objs;
    m_objs.reserve(total_objs);
    struct kea_obj_access_hdr *prev_dyn = NULL;
    for (unsigned i = 0; i < total_objs; ++i) {
        createObj(i + 1);
        if (i >= num_static_objs) {
            if (m_hdr.dyn_objs == NULL) {
                m_hdr.dyn_objs = m_hdr.objs[i];
            } else {
                prev_dyn->next = m_hdr.objs[i];
            }
            prev_dyn = m_hdr.objs[i];
        }
    }
    
}

void TestModelCreator::createObj(unsigned obj_id) {
    enum kea_data_types types[] = {
        KEA_TYPE_INT8,
        KEA_TYPE_UINT8,
        KEA_TYPE_INT16,
        KEA_TYPE_UINT16,
        KEA_TYPE_INT32,
        KEA_TYPE_UINT32,
        KEA_TYPE_INT64,
        KEA_TYPE_UINT64
    };

    if (m_cur_obj_id >= max_models) {
        throw std::runtime_error("Exceeded maximum number of objects");
    }
    m_objs.emplace_back(std::make_unique<TestObjCreator>(
        obj_id, KEA_COLLECTION_TYPE_SCALAR, types[obj_id % (sizeof(types) / sizeof(types[0]))], 0));
    m_c_objs[m_cur_obj_id++] = &m_objs.back()->m_prim;
}

TestModelCreator &TestModelGen::addModel(unsigned num_static_objs, unsigned num_dyn_objs) {
    auto &m = m_models.emplace_back(std::make_unique<TestModelCreator>(
        m_next_model_id++, num_static_objs, num_dyn_objs));
    return *m;
}

TestModelCreator &TestModelGen::getModel(unsigned idx) {
    if (idx >= m_models.size()) {
        throw std::out_of_range("Model index out of range");
    }
    auto &m = m_models[idx];
    return *m;
}