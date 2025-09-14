#ifndef __KEA_CMD_PROC_PRIV_IF_H
#define __KEA_CMD_PROC_PRIV_IF_H

int kea_cmd_unsupported(struct kea_stream *requestor, char *req, unsigned req_len);

#define __WEAK_ALIAS_UNSUPPORTED __attribute__((weak, alias("kea_cmd_unsupported")))
#define __PACKED 

// int kea_cmd_proc_read(struct kea_stream *requestor, char *req, unsigned req_len) __WEAK_ALIAS_UNSUPPORTED;
// int kea_cmd_connect(struct kea_stream *requestor, char *req, unsigned req_len) __WEAK_ALIAS_UNSUPPORTED;

void kea_cmd_processor(struct kea_stream *requestor, const char *req_pkt, unsigned req_len);

struct kea_msg_reqs {
    unsigned num_reqs: 8;
    unsigned 
    char reqs[0];
};

struct kea_req_msg_hdr {
    unsigned version: 8;
    unsigned op_code: 8;
    unsigned transaction_id: 8;
    unsigned seq_num: 8;
    unsigned len: 8;
    char body[0];
};

struct kea_rsp_msg_hdr {
    unsigned version: 8;
    unsigned op_code: 8;
    unsigned transaction_id: 8;
    unsigned status: 8;
    unsigned seq_num: 8;
    unsigned len: 8;
    char body[0];
};

#define KEA_REQ_MSG_HDR_SZ  sizeof(struct kea_req_msg_hdr)
#define KEA_RSP_MSG_HDR_SZ  sizeof(struct kea_rsp_msg_hdr)

#endif /* __KEA_CMD_PROC_PRIV_IF_H */