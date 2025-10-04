#ifndef __KEA_CMD_DEFS_PRIV_IF_H
#define __KEA_CMD_DEFS_PRIV_IF_H

struct kea_command_request {
    struct kea_stream *stream;
    const char *req_pkt;
    unsigned req_len;
};

void kea_cmd_get_all_models_proc_req(struct kea_command_request* cmd);
void kea_cmd_get_model_schema_proc_req(struct kea_command_request* cmd);

#endif /* __KEA_CMD_DEFS_PRIV_IF_H */


