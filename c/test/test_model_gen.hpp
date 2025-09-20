#include <string>
#include <vector>

#include "kea/kea.h"

class TestObjCreator {
public:
    TestObjCreator(
        unsigned obj_id, enum kea_data_collection_types collection_type, enum kea_data_types type, unsigned num_elems);
    ~TestObjCreator();

    union {
        struct kea_obj_access_hdr m_prim;
        struct kea_obj_static_array m_static_arr;
        struct kea_obj_custom m_custom;
    };
};

class TestModelGen {
public:
    TestModelGen();
    ~TestModelGen();

    void addModel(unsigned num_static_objs, unsigned num_dyn_objs);

    std::vector<struct kea_model_hdr*> m_models;
    int m_next_model_id = 1;
};