#include <stdbool.h>
#include <string.h>

#include "kea/kea.h"
#include "kea/kea_model.h"
#include "kea/kea_cmds.h"
#include "commands/kea_cmd_processor.h"

#include "stream/kea_stream_helpers.h"

#define GET_MODELS_RESP_VERSION 0

static char rsp_buf[CONFIG_KEA_MAX_REQ_BUF_SIZE];

enum buf_write_status {
    BUF_WRITE_OK_SPACE_REMAINING = 0,
    BUF_SEND_WRITE_PARTIAL,
    BUF_SEND_WRITE_SUCCESS
};

static enum buf_write_status safe_buf_write(
    char **buf_ptr, const char *end_ptr, const void *src, int data_len, unsigned *offset)
{
    int free_space;
    char *data_to_send = (char *)src;
    unsigned off = 0;
    bool allow_partial = offset != NULL;
    enum buf_write_status rc;

    if (allow_partial) {
        if (*offset < (unsigned)data_len) {
            off = *offset;
            data_to_send += off;
            data_len -= off;
        } else if (*offset >= (unsigned)data_len) {
            return BUF_WRITE_OK_SPACE_REMAINING;
        }
    }
    
    free_space = (int)end_ptr - (int)*buf_ptr;

    if (free_space <= 0 || (!allow_partial && free_space < data_len))
        return BUF_SEND_WRITE_PARTIAL;
    
    if (allow_partial && free_space < data_len) {
        data_len = free_space;
        rc = BUF_SEND_WRITE_PARTIAL;
    } else {
        rc = BUF_SEND_WRITE_SUCCESS;
    }

    memcpy(*buf_ptr, data_to_send, data_len);
    *buf_ptr += data_len;
    free_space = (int)end_ptr - (int)*buf_ptr;
    if (allow_partial) {
        *offset += data_len;
    }
    
    return free_space > 0 ? BUF_WRITE_OK_SPACE_REMAINING : rc;
}

static void kea_cmd_get_all_models(struct kea_stream *rq, const void *req_pkt, unsigned req_len)
{
    bool do_include_names;
    unsigned char payload_len;
    unsigned char num_models;
    struct kea_model_iter it;
    struct kea_model_hdr *model;
    char *end_ptr = &rsp_buf[CONFIG_KEA_MAX_REQ_BUF_SIZE];
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

        if (safe_buf_write(&ptr, end_ptr, &model->model_id, sizeof(cur_rsp_model->model_id), NULL) != BUF_WRITE_OK_SPACE_REMAINING) {
            /* No space left in buffer */
            if (!kea_stream_write(rq, rsp_buf, sizeof(rsp->hdr) + payload_len)) {
                // Failed to write response
                return;
            }

            payload_len = 0;
            ptr = &rsp_buf[sizeof(rsp->hdr)];
            rsp->hdr.seq_num++;
        }
    }
    
}

void kea_cmd_processor(struct kea_stream *requestor, const char *req_pkt, unsigned req_len)
{
    const struct kea_cmd_all_models_req *req = (const struct kea_cmd_all_models_req *)req_pkt;
    switch (req->hdr.op_code)
    {
    case KEA_CMD_GET_ALL_MODELS:
        kea_cmd_get_all_models(requestor, req, req_len);
        break;
    
    default:
        break;
    }
}