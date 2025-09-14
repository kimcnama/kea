#include <string.h>
#include <stdbool.h>
#include "kea/kea.h"
#include "kea/kea_cmds.h"
#include "stream/kea_stream_helpers.h"

static char req_buf[CONFIG_KEA_MAX_REQ_BUF_SIZE];
unsigned char cur_trans_id = 0;

bool kea_req_get_all_models(struct kea_stream *stream, bool do_include_names)
{
    struct kea_cmd_all_models_req *req = (struct kea_cmd_all_models_req *)req_buf;

    if (kea_is_stream_invalid(stream))
        return false;
    
    memset(req_buf, 0x0, sizeof(req_buf));
    req->hdr.version = KEA_PRTOCOL_VERSION;
    req->hdr.op_code = KEA_CMD_GET_ALL_MODELS;
    req->hdr.transaction_id = cur_trans_id++;
    req->hdr.len = 0;
    req->do_include_names = do_include_names ? 1 : 0;
    return sizeof(struct kea_cmd_all_models_req) == 
        stream->write(stream->metadata, req_buf, sizeof(struct kea_cmd_all_models_req));
}