#ifndef __KEA_MODEL_MGR_PRIV_IF_H
#define __KEA_MODEL_MGR_PRIV_IF_H

#include <stdbool.h>

bool kea_model_iter_init(struct kea_model_iter *iter);
bool kea_model_iter(struct kea_model_iter *iter, struct kea_model_hdr **model);
unsigned char kea_get_num_registered_models(void);
struct kea_model_hdr *kea_model_get(unsigned model_id);

#endif /* __KEA_MODEL_MGR_PRIV_IF_H */