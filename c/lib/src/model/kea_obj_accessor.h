#ifndef __KEA_OBJ_ACCESSOR_PRIV_IF_H
#define __KEA_OBJ_ACCESSOR_PRIV_IF_H

#include <stdbool.h>

struct model_attr_iter {
    struct kea_model_hdr *model;
    bool is_static;
    struct kea_obj_access_hdr *dyn_cur;
    unsigned static_idx;
};

bool kea_model_attr_iter_init(struct model_attr_iter *iter, struct kea_model_hdr *model);
bool kea_model_attr_iter_next(struct model_attr_iter *iter, struct kea_obj_access_hdr **attr);

struct kea_obj_access_hdr *kea_obj_accessor_get(struct kea_model_hdr *model, unsigned char obj_id);

int get_size_of_kea_obj(struct kea_obj_access_hdr *obj);

#endif /* __KEA_OBJ_ACCESSOR_PRIV_IF_H */