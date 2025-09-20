#ifndef __KEA_CMD_DEFS_PRIV_IF_H
#define __KEA_CMD_DEFS_PRIV_IF_H

void kea_cmd_get_all_models_proc_req(
    struct kea_stream *rq, const void *req_pkt, unsigned req_len, void *rsp_buf, unsigned max_rsp_buf_len);

#endif /* __KEA_CMD_DEFS_PRIV_IF_H */


