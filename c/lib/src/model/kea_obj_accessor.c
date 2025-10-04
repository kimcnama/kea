#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "kea/kea_model.h"
#include "model/kea_obj_accessor.h"

bool kea_model_attr_iter_init(struct model_attr_iter *iter, struct kea_model_hdr *model)
{
    if (!iter || !model)
        return false;
    
    iter->model = model;
    iter->is_static = (model->num_static_objs > 0);
    iter->static_idx = 0;
    iter->dyn_cur = model->dyn_objs;
    return true;
}

bool kea_model_attr_iter_next(struct model_attr_iter *iter, struct kea_obj_access_hdr **attr)
{
    if (!iter || !attr)
        return false;
    
    if (iter->is_static) {
        if (iter->static_idx < iter->model->num_static_objs) {
            *attr = iter->model->objs[iter->static_idx++];
            return true;
        } else {
            iter->is_static = false;
        }
    }
    
    if (!iter->is_static) {
        if (iter->dyn_cur) {
            *attr = iter->dyn_cur;
            iter->dyn_cur = iter->dyn_cur->next;
            return true;
        }
    }

    return false;
}

struct kea_obj_access_hdr *kea_obj_accessor_get(struct kea_model_hdr *model, unsigned char obj_id)
{
    struct kea_obj_access_hdr *tmp = NULL;
    int i;
    
    if (!model || !model->objs)
        return NULL;
    
    /* Search compiled objects for ID */
    if (model->num_static_objs > 0 && obj_id < model->num_static_objs) {
        
        tmp = model->objs[obj_id];
        if (tmp && tmp->obj_id == obj_id)
            return tmp;
        
        for (i = 0; i < model->num_static_objs; ++i) {
            if (model->objs[i]->obj_id == obj_id)
                return model->objs[i];
        }
    }

    /* Search dynamically added objects for ID */
    if (model->num_dyn_objs == 0)
        return NULL;
    
    tmp = model->dyn_objs;
    while (tmp) {
        if (tmp->obj_id == obj_id)
            return tmp;
        tmp = tmp->next;
    }
    
    return NULL;
}

int get_size_of_kea_obj(struct kea_obj_access_hdr *obj)
{
    if (!obj)
        return -1;
    
    switch (obj->collection_type) {
    case KEA_COLLECTION_TYPE_SCALAR:
        switch (obj->type) {
        case KEA_TYPE_INT8:
        case KEA_TYPE_UINT8:
            return sizeof(uint8_t);
        case KEA_TYPE_INT16:
        case KEA_TYPE_UINT16:
            return sizeof(uint16_t);
        case KEA_TYPE_INT32:
        case KEA_TYPE_UINT32:
            return sizeof(uint32_t);
        case KEA_TYPE_INT64:
        case KEA_TYPE_UINT64:
            return sizeof(uint64_t);
        case KEA_TYPE_FLOAT:
            return sizeof(float);
        case KEA_TYPE_DOUBLE:
            return sizeof(double);
        default:
            return -1;
        }
    case KEA_COLLECTION_TYPE_ARRAY_STATIC:
        {
            struct kea_obj_static_array *arr = (struct kea_obj_static_array *)obj;
            return arr->elem_size * arr->num_elems;
        }
    case KEA_COLLECTION_TYPE_ARRAY_DYNAMIC:
    case KEA_COLLECTION_TYPE_CUSTOM:
        {
            struct kea_obj_custom *arr = (struct kea_obj_custom *)obj;
            return arr->size(arr->hdr.self);
        }
    default:
        return -1;
    }
}