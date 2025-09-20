#include <string.h>
#include "test_model_gen.hpp"

TestObjCreator::TestObjCreator(
    unsigned obj_id, enum kea_data_collection_types collection_type, enum kea_data_types type, unsigned num_elems) {
    
    struct kea_obj_access_hdr *h = &m_prim;
    m_prim.obj_id = obj_id;
    m_prim.init_status = 1;
    m_prim.collection_type = collection_type;
    m_prim.type = type;
    std::string name = "obj_" + std::to_string(obj_id);
    m_prim.name = strdup(name.c_str());
    m_prim.next = nullptr;
    m_prim.self = nullptr; // Placeholder

    if (collection_type == KEA_COLLECTION_TYPE_SCALAR) {
        m_prim.self = malloc(sizeof(long long)); // Just be lazy to begin with
        if (!m_prim.self) {
            throw std::bad_alloc();
        }

        // Fill m_prim.self with random data
        long long random_value = ((long long)rand() << 32) | rand();
        memcpy(m_prim.self, &random_value, sizeof(long long));
    }
}

TestObjCreator::~TestObjCreator() {
    struct kea_obj_access_hdr *h = &m_prim;
    
    if (h->name)
        free(h->name);
    if (h->self)
        free(h->self);
    
}

TestModelGen::TestModelGen() {
    m_models.clear();
    m_next_model_id;
}

TestModelGen::~TestModelGen() {
    for (auto model : m_models) {
        if (model->name)
            free((void*)model->name);
        free(model);
    }
}

void TestModelGen::addModel(unsigned num_static_objs, unsigned num_dyn_objs) {
    struct kea_model_hdr *model = (struct kea_model_hdr*)calloc(1, sizeof(struct kea_model_hdr));
    if (!model) {
        throw std::bad_alloc();
    }
    
    model->model_id = m_next_model_id;
    std::string name = "test_model_" + std::to_string(m_next_model_id);
    model->name = strdup(name.c_str());
    for (int i = 0; i < MODEL_HASH_SZ; ++i)
        model->hash[i] = m_next_model_id; // Placeholder hash
    m_next_model_id++;

    model->objs = nullptr;
    model->num_static_objs = num_static_objs;
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
    
    model->objs = 
        (num_static_objs > 0) ? ((struct kea_obj_access_hdr **)calloc(num_static_objs, sizeof(struct kea_obj_access_hdr*))) : nullptr;

    for (unsigned i = 0; i < num_static_objs; ++i) {
        TestObjCreator obj(i + 1, KEA_COLLECTION_TYPE_SCALAR, types[i % (sizeof(types) / sizeof(types[0]))], 0);
        model->objs[i] = &obj.m_prim;
    }
    
    struct kea_obj_access_hdr *last_obj = nullptr;
    model->dyn_objs = nullptr;
    model->num_dyn_objs = num_dyn_objs;
    for (unsigned i = 0; i < num_dyn_objs; ++i) {
        TestObjCreator obj(i + 1, KEA_COLLECTION_TYPE_SCALAR, types[i % (sizeof(types) / sizeof(types[0]))], 0);
        if (last_obj)
            last_obj->next = &obj.m_prim;
        else
            model->dyn_objs = &obj.m_prim;
        last_obj = &obj.m_prim;
    }

    m_models.push_back(model);
}
