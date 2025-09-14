#ifndef __KEA_OBJ_ACCESSOR_PRIV_IF_H
#define __KEA_OBJ_ACCESSOR_PRIV_IF_H

#include <stdbool.h>

struct kea_obj_access_hdr *kea_obj_accessor_get(struct kea_model_hdr *model, unsigned char obj_id);

#endif /* __KEA_OBJ_ACCESSOR_PRIV_IF_H */