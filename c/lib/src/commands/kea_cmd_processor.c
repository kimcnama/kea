#include <stdbool.h>
#include <string.h>

#include "kea/kea.h"
#include "kea/kea_model.h"
#include "kea/kea_cmds.h"
#include "commands/kea_cmd_processor.h"
#include "commands/kea_cmd_defs.h"

#include "stream/kea_stream_helpers.h"



bool safe_buf_write_and_send(
    struct kea_stream *stream, struct kea_rsp_hdr *hdr, char **buf_ptr, const char *end_ptr, const void *src, int data_len)
{
    int copied_bytes, bytes_written;
    int free_space = (int)end_ptr - (int)*buf_ptr - 1;
    char *src_ptr = (char *)src;

    while (data_len > 0 || free_space == 0) {

        if (free_space > 0) {
            copied_bytes = data_len < free_space ? data_len : free_space;
            memcpy(*buf_ptr, src_ptr, copied_bytes);
            *buf_ptr += copied_bytes;
            data_len -= copied_bytes;
            src_ptr += copied_bytes;
            free_space -= copied_bytes;
            hdr->len += copied_bytes;
        }

        if (free_space == 0) {
            bytes_written = kea_stream_write(stream, hdr, sizeof(*hdr) + hdr->len);
            if (bytes_written < (unsigned)(*buf_ptr - (char *)hdr)) {
                // Failed to write response
                return false;
            }

            *buf_ptr = &((char *)hdr)[sizeof(struct kea_cmd_all_models_rsp)];
            free_space = (int)end_ptr - (int)*buf_ptr - 1;
            hdr->seq_num++;
            hdr->len = 0;
        }
    }
    
    return true;
}

enum buf_write_status safe_buf_write(
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

void kea_cmd_processor(struct kea_stream *requestor, const char *req_pkt, unsigned req_len)
{
    const struct kea_cmd_all_models_req *req = (const struct kea_cmd_all_models_req *)req_pkt;

    char rsp_buf[CONFIG_KEA_MAX_REQ_BUF_SIZE];

    switch (req->hdr.op_code)
    {
    case KEA_CMD_GET_ALL_MODELS:
        kea_cmd_get_all_models_proc_req(requestor, req, req_len, rsp_buf, sizeof(rsp_buf));
        break;
    
    default:
        break;
    }
}