#include <stddef.h>

#include "kea/kea_model.h"
#include "model/kea_obj_accessor.h"

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