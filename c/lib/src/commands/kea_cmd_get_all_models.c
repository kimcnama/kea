#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "kea/kea.h"
#include "kea/kea_cmds.h"
#include "kea/kea_model.h"
#include "stream/kea_stream_helpers.h"
#include "commands/kea_cmd_processor.h"
#include "commands/kea_cmd_defs.h"

#define GET_MODELS_RESP_VERSION     (0)

bool kea_req_get_all_models(
    struct kea_stream *stream, bool do_include_names, unsigned char trans_id)
{
    struct kea_cmd_all_models_req req = {0};

    if (kea_is_stream_invalid(stream))
        return -EINVAL;

    req.hdr.version = KEA_PRTOCOL_VERSION;
    req.hdr.op_code = KEA_CMD_GET_ALL_MODELS;
    req.hdr.transaction_id = trans_id;
    req.hdr.len = 0;
    req.do_include_names = do_include_names ? 1 : 0;
    return sizeof(struct kea_cmd_all_models_req) == 
        kea_stream_write(stream, &req, sizeof(struct kea_cmd_all_models_req));
}

unsigned kea_cmd_get_all_models_decode_rsp(
    const void *rsp_buf, unsigned resp_len, struct kea_rsp_model_meta **models_info, unsigned max_model_array_size) {
    
    char *ptr;
    struct kea_cmd_all_models_rsp *rsp = (struct kea_cmd_all_models_rsp *)rsp_buf;
    unsigned num_models = 0;
    
    ptr = &rsp_buf[sizeof(rsp->hdr) + sizeof(rsp->rsp_version) + sizeof(rsp->num_models)];

    while (num_models < max_model_array_size && num_models < rsp->num_models) {
        struct kea_rsp_model_meta *cur_model = (struct kea_rsp_model_meta *)ptr;
        models_info[num_models++] = cur_model;
        ptr += sizeof(cur_model->model_id) + sizeof(cur_model->name_len) + cur_model->name_len;
    }
    
    return num_models;
}

void kea_cmd_get_all_models_proc_req(
    struct kea_stream *rq, const void *req_pkt, unsigned req_len, void *rsp_buf, unsigned max_rsp_buf_len)
{
    bool do_include_names, is_ok;
    unsigned char num_models, str_len;
    struct kea_model_iter it;
    struct kea_model_hdr *model;
    char *ptr;
    char *end_ptr = &((char*)rsp_buf)[max_rsp_buf_len];

    const struct kea_cmd_all_models_req *req = (const struct kea_cmd_all_models_req *)req_pkt;
    struct kea_cmd_all_models_rsp *rsp = (struct kea_cmd_all_models_rsp *)rsp_buf;
    
    do_include_names = req->do_include_names ? true : false;

    rsp->hdr.version = req->hdr.version;
    rsp->hdr.op_code = req->hdr.op_code;
    rsp->hdr.transaction_id = req->hdr.transaction_id;
    rsp->hdr.status = 0; // Success
    rsp->hdr.seq_num = 0;
    
    rsp->rsp_version = GET_MODELS_RESP_VERSION;
    rsp->num_models = kea_get_num_registered_models();
    rsp->hdr.len = sizeof(rsp->rsp_version) + sizeof(rsp->num_models);

    ptr = &rsp_buf[sizeof(rsp->hdr) + sizeof(rsp->rsp_version) + sizeof(rsp->num_models)];

    kea_model_iter_init(&it);
    while (kea_model_iter(&it, &model)) {
        
        struct kea_rsp_model_meta *cur_rsp_model = (struct kea_rsp_model_meta *)ptr;
        
        /* add model id */
        is_ok = safe_buf_write_and_send(rq, &rsp->hdr, &ptr, end_ptr, &model->model_id, sizeof(cur_rsp_model->model_id));
        if (!is_ok) {
            // Failed to write response
            return;
        }

        /* string length of name */
        str_len = do_include_names ? (char)strlen(model->name) : 0;
        is_ok = safe_buf_write_and_send(rq, &rsp->hdr, &ptr, end_ptr, &str_len, sizeof(cur_rsp_model->name_len));
        if (!is_ok) {
            // Failed to write response
            return;
        }

        /* Add name of model */
        if (do_include_names && str_len > 0) {
            is_ok = safe_buf_write_and_send(rq, &rsp->hdr, &ptr, end_ptr, model->name, str_len);
            if (!is_ok) {
                // Failed to write response
                return;
            }
        }
    }
    // Final write of any remaining data
    if (ptr > (char *)&rsp->hdr + sizeof(rsp->hdr)) {
        (void)kea_stream_write(rq, rsp, rsp->hdr.len + sizeof(rsp->hdr));
    }
}
