#include <stdbool.h>
#include <string.h>

#include "kea/kea.h"
#include "kea/kea_model.h"
#include "kea/kea_cmds.h"
#include "commands/kea_cmd_processor.h"

#include "stream/kea_stream_helpers.h"

static char rsp_buf[CONFIG_KEA_MAX_REQ_BUF_SIZE];

enum buf_write_status {
    BUF_WRITE_OK_SPACE_REMAINING = 0,
    BUF_SEND_WRITE_PARTIAL,
    BUF_SEND_WRITE_SUCCESS
};

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
    switch (req->hdr.op_code)
    {
    case KEA_CMD_GET_ALL_MODELS:
        kea_cmd_get_all_models(requestor, req, req_len);
        break;
    
    default:
        break;
    }
}