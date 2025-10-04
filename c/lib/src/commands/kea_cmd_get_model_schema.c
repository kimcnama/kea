#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "kea/kea.h"
#include "kea/kea_cmds.h"
#include "kea/kea_model.h"
#include "stream/kea_stream_helpers.h"
#include "commands/kea_cmd_processor.h"
#include "commands/kea_cmd_defs.h"
#include "model/kea_obj_accessor.h"
#include "model/kea_model_mgr.h"

bool kea_req_get_model_schema(
    struct kea_stream *stream, uint16_t model_id, bool do_include_names, unsigned trans_id)
{
    struct kea_cmd_get_model_schema_req req = {0};
    
    if (kea_is_stream_invalid(stream))
        return -EINVAL;

    req.hdr.version = KEA_PRTOCOL_VERSION;
    req.hdr.op_code = KEA_CMD_GET_MODEL_SCHEMA;
    req.hdr.transaction_id = trans_id;
    req.hdr.len = sizeof(struct kea_cmd_get_model_schema_req) - sizeof(struct kea_req_hdr);
    req.model_id = model_id;
    req.do_include_names = do_include_names ? 1 : 0;
    return sizeof(struct kea_cmd_get_model_schema_req) == 
        kea_stream_write(stream, &req, sizeof(struct kea_cmd_get_model_schema_req));
}

int kea_cmd_get_model_schema_decode_rsp(
    const void *rsp_buf, unsigned resp_len, struct kea_cmd_model_attr **attrs, unsigned max_attr_array_size)
{
    struct kea_cmd_get_model_schema_rsp *response = (struct kea_cmd_get_model_schema_rsp *)rsp_buf;
    char *rsp = (char *)&response->attrs[0];
    
    int ret_attrs = 0;

    for (int i = 0; i < max_attr_array_size && ret_attrs < response->num_attrs; ++i) {
        struct kea_cmd_model_attr *cur_attr = (struct kea_cmd_model_attr *)rsp;
        attrs[ret_attrs++] = cur_attr;
        rsp += sizeof(struct kea_cmd_model_attr) + cur_attr->name_len;
    }
    return ret_attrs;
}

void kea_cmd_get_model_schema_proc_req(struct kea_command_request* cmd)
{   
    struct model_attr_iter it;
    struct kea_obj_access_hdr *attr;
    struct kea_model_hdr *model;
    unsigned char tmp;

    const struct kea_cmd_get_model_schema_req *request = (const struct kea_cmd_get_model_schema_req *)cmd->req_pkt;
    bool do_include_names = (request->do_include_names != 0);

    model = kea_model_get(request->model_id);
    if (!model) {
        // Model not found, send error response
        kea_stream_send_err_rsp(cmd->stream, KEA_CMD_STATUS_MODEL_INVALID, KEA_CMD_GET_MODEL_SCHEMA, request->hdr.transaction_id);
        return;
    }

    if (!kea_model_attr_iter_init(&it, model)) {
        // Failed to init iterator, send error response
        kea_stream_send_err_rsp(cmd->stream, KEA_CMD_STATUS_ERR, KEA_CMD_GET_MODEL_SCHEMA, request->hdr.transaction_id);
        return;
    }

    kea_stream_init_rsp_buf(cmd->stream, KEA_CMD_GET_MODEL_SCHEMA, request->hdr.transaction_id);
    /* model_id */
    if (!kea_stream_write_or_err(cmd->stream, &model->model_id, sizeof(uint16_t))) return;
    /* num_static_objs */
    if (!kea_stream_write_or_err(cmd->stream, &model->num_static_objs, sizeof(unsigned char))) return;
    /* num_attrs */
    tmp = model->num_static_objs + model->num_dyn_objs;
    if (!kea_stream_write_or_err(cmd->stream, &tmp, sizeof(unsigned char))) return;

    /* attrs[] */
    while (kea_model_attr_iter_next(&it, &attr)) {
        
        /* obj_id */
        if (!kea_stream_write_or_err(cmd->stream, &attr->obj_id, sizeof(unsigned char))) return;

        /* collection_type + type */
        tmp = (unsigned char)((attr->collection_type << 4) | (attr->type & 0x0F));
        if (!kea_stream_write_or_err(cmd->stream, &tmp, sizeof(unsigned char))) return;

        /* size_bytes */
        int size_bytes = get_size_of_kea_obj(attr);
        uint16_t size_bytes_u16 = (size_bytes < 0) ? 0 : (uint16_t)size_bytes;
        if (!kea_stream_write_or_err(cmd->stream, &size_bytes_u16, sizeof(uint16_t))) return;

        /* name_len */
        unsigned name_len = (do_include_names && attr->name) ? strlen(attr->name) : 0;
        tmp = (unsigned char)name_len;
        if (!kea_stream_write_or_err(cmd->stream, &tmp, sizeof(unsigned char))) return;

        /* name */
        if (name_len > 0) {
            if (!kea_stream_write_or_err(cmd->stream, attr->name, name_len)) return;
        }
    }

    // Final write of any remaining data
    if (!kea_stream_send_rsp_buf_if_nonempty(cmd->stream)) {
        (void)kea_stream_send_initd_err_rsp(cmd->stream, KEA_CMD_STATUS_STREAM_ERR);
    }
}