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

void kea_cmd_processor(struct kea_stream *requestor, const char *req_pkt, unsigned req_len)
{
    const struct kea_req_hdr *req = (const struct kea_req_hdr *)req_pkt;
    struct kea_command_request request = {
        .stream = requestor,
        .req_pkt = req_pkt,
        .req_len = req_len
    };

    switch (req->op_code)
    {
    case KEA_CMD_GET_ALL_MODELS:
        kea_cmd_get_all_models_proc_req(&request);
        break;
    case KEA_CMD_GET_MODEL_SCHEMA:
        kea_cmd_get_model_schema_proc_req(&request);
        break;
    default:
        break;
    }
}