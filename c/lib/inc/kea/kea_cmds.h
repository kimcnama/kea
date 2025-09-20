#ifndef __KEA_CMDS_PUBLIC_IF_H
#define __KEA_CMDS_PUBLIC_IF_H

#include <stdint.h>
#include <stdbool.h>

enum kea_cmds {
    KEA_CMD_READ = 1,
    KEA_CMD_WRITE = 2,
    KEA_CMD_GET_ALL_MODELS = 3,
    KEA_CMD_GET_MODEL_INFO = 4
};

struct kea_req_hdr {
    unsigned char version;
    unsigned char op_code;
    unsigned char transaction_id;
    unsigned short len;
} __attribute__((packed));

struct kea_rsp_hdr {
    unsigned char version;
    unsigned char op_code;
    unsigned char transaction_id;
    unsigned char status;
    unsigned char seq_num;
    unsigned short len;
} __attribute__((packed));

struct kea_cmd_all_models_req {
    struct kea_req_hdr hdr;
    uint8_t do_include_names;
} __attribute__((packed));

struct kea_rsp_model_meta {
    unsigned char model_id;
    char name_len;
    char name[0];
} __attribute__((packed));

struct kea_cmd_all_models_rsp {
    struct kea_rsp_hdr hdr;
    unsigned char rsp_version;
    unsigned char num_models;
    struct kea_rsp_model_meta models[0];
} __attribute__((packed));

struct kea_cmd_model_attr {
    unsigned char obj_id;
    unsigned collection_type: 4;
    unsigned type: 4;
    uint16_t size_bytes;
    unsigned char name_len;
    char name[0];
} __attribute__((packed));

struct kea_cmd_get_model_schema {
    struct kea_rsp_hdr hdr;
    uint16_t model_id;
    unsigned char num_attrs;
    struct kea_cmd_model_attr attrs[0];
} __attribute__((packed));

bool kea_req_get_all_models(
    struct kea_stream *stream, bool do_include_names, unsigned char trans_id);

unsigned kea_cmd_get_all_models_decode_rsp(
    const void *rsp_buf, unsigned resp_len, struct kea_rsp_model_meta **models_info, unsigned max_model_array_size);

#endif /* __KEA_CMDS_PUBLIC_IF_H */