#include <stddef.h>
#include <errno.h>
#include "kea/kea_model.h"
#include "model/kea_model_mgr.h"

// Temp add in bad DS 
#define MAX_MODELS 16
static struct {
    unsigned char num_registered;
    struct kea_model_hdr *registered_models[MAX_MODELS];
} mgr;

bool kea_model_iter_init(struct kea_model_iter *iter)
{
    if (!iter)
        return false;
    iter->cur_idx = 0;
    return true;
}

bool kea_model_iter(struct kea_model_iter *iter, struct kea_model_hdr **model)
{
    struct kea_model_hdr *m;
    bool is_valid_model;
    
    if (!iter || iter->cur_idx >= MAX_MODELS)
        return false;
    
    m = mgr.registered_models[iter->cur_idx++];
    *model = m;
    is_valid_model = (m != NULL);
    return is_valid_model;
}

unsigned char kea_get_num_registered_models(void)
{
    unsigned char count = 0;
    
    for (int i = 0; i < MAX_MODELS; ++i) {
        if (mgr.registered_models[i])
            ++count;
    }
    
    return count;
}

struct kea_model_hdr *kea_model_get(unsigned model_id)
{
    if (model_id == 0 || model_id > MAX_MODELS)
        return NULL;
    
    return mgr.registered_models[model_id - 1];
}

int kea_model_get_next_free_model_id(void)
{
    for (int i = 0; i < MAX_MODELS; ++i) {
        if (!mgr.registered_models[i])
            return i + 1;
    }
    
    return -ENOMEM;
}

int kea_model_register(struct kea_model_hdr *model, bool use_first_free_id)
{
    int rc;
    
    if (!model || !model->objs || mgr.num_registered >= MAX_MODELS)
        return -1;

    if (use_first_free_id) {
        rc = kea_model_get_next_free_model_id();
        
        if (rc < 0)
            return rc;
        
        model->model_id = (unsigned char)rc;
    } else {        
        if (model->model_id == 0 || kea_model_get(model->model_id) != NULL)
            return -EINVAL;
    }

    mgr.registered_models[model->model_id - 1] = model;
    return 0;
}

int kea_model_mgr_init(void)
{
    memset(&mgr, 0x0, sizeof(mgr));
    return 0;
}