#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "kea/kea.h"
#include "kea/kea_cmds.h"
#include "kea/kea_model.h"
#include "stream/kea_stream_helpers.h"
#include "commands/kea_cmd_processor.h"

#define GET_MODELS_RESP_VERSION     (0)

int kea_req_get_all_models(
    struct kea_stream *stream, bool do_include_names, unsigned char trans_id, void *buf, unsigned max_buf_len)
{
    struct kea_cmd_all_models_req *req = (struct kea_cmd_all_models_req *)buf;

    if (max_buf_len < sizeof(struct kea_cmd_all_models_req))
        return -ENOMEM;

    if (kea_is_stream_invalid(stream) || !buf)
        return -EINVAL;
    
    memset(req, 0x0, max_buf_len);
    req->hdr.version = KEA_PRTOCOL_VERSION;
    req->hdr.op_code = KEA_CMD_GET_ALL_MODELS;
    req->hdr.transaction_id = trans_id;
    req->hdr.len = 0;
    req->do_include_names = do_include_names ? 1 : 0;
    return stream->write(stream->metadata, req, sizeof(struct kea_cmd_all_models_req));
}

static void kea_cmd_get_all_models(
    struct kea_stream *rq, const void *req_pkt, unsigned req_len, void *rsp_buf, unsigned max_rsp_buf_len)
{
    bool do_include_names;
    unsigned char payload_len;
    unsigned char num_models;
    struct kea_model_iter it;
    struct kea_model_hdr *model;
    char *end_ptr = &((char*)rsp_buf)[max_rsp_buf_len];
    char *ptr;
    unsigned offset;
    unsigned char str_len;
    enum buf_write_status status;

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
    payload_len = sizeof(rsp->rsp_version) + sizeof(rsp->num_models);

    ptr = &rsp_buf[sizeof(rsp->hdr) + sizeof(rsp->rsp_version) + sizeof(rsp->num_models)];

    kea_model_iter_init(&it);
    while (kea_model_iter(&it, &model)) {
        
        struct kea_rsp_model_meta *cur_rsp_model = (struct kea_rsp_model_meta *)ptr;
        
        /* add model id */
        do {
            status = safe_buf_write(&ptr, end_ptr, &model->model_id, sizeof(cur_rsp_model->model_id), NULL);
            if (status != BUF_WRITE_OK_SPACE_REMAINING) {
                /* No space left in buffer */
                if (!kea_stream_write(rq, rsp_buf, sizeof(rsp->hdr) + payload_len)) {
                    // Failed to write response
                    return;
                }

                payload_len = 0;
                ptr = &rsp_buf[sizeof(rsp->hdr)];
                rsp->hdr.seq_num++;
            }
        } while (status != BUF_WRITE_OK_SPACE_REMAINING);

        /* string length of name */
        do {
            str_len = do_include_names ? (char)strlen(model->name) : 0;
            status = safe_buf_write(&ptr, end_ptr, &str_len, sizeof(cur_rsp_model->name_len), NULL);
            if (status != BUF_WRITE_OK_SPACE_REMAINING) {
                /* No space left in buffer */
                if (!kea_stream_write(rq, rsp_buf, sizeof(rsp->hdr) + payload_len)) {
                    // Failed to write response
                    return;
                }

                payload_len = 0;
                ptr = &rsp_buf[sizeof(rsp->hdr)];
                rsp->hdr.seq_num++;
            }
        } while (status != BUF_WRITE_OK_SPACE_REMAINING);

        /* Add name of model */
        if (do_include_names && str_len > 0) {
            offset = 0;
            
            do {
                status = safe_buf_write(&ptr, end_ptr, model->name, str_len, &offset);
                
                if (status == BUF_SEND_WRITE_PARTIAL) {
                    payload_len += offset;
                } else if (status == BUF_SEND_WRITE_SUCCESS) {
                    payload_len += str_len;
                }
                
                if (status != BUF_WRITE_OK_SPACE_REMAINING) {
                    /* No space left in buffer */
                    if (!kea_stream_write(rq, rsp_buf, sizeof(rsp->hdr) + payload_len)) {
                        // Failed to write response
                        return;
                    }

                    payload_len = 0;
                    ptr = &rsp_buf[sizeof(rsp->hdr)];
                    rsp->hdr.seq_num++;
                }
            } while (status != BUF_WRITE_OK_SPACE_REMAINING);
        }

    }
    
}
