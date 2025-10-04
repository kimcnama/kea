#ifndef __KEA_CMDS_PUBLIC_IF_H
#define __KEA_CMDS_PUBLIC_IF_H

#include <stdint.h>
#include <stdbool.h>

enum kea_cmds {
    KEA_CMD_READ = 1,
    KEA_CMD_WRITE = 2,
    KEA_CMD_GET_ALL_MODELS = 3,
    KEA_CMD_GET_MODEL_SCHEMA = 4
};

enum kea_cmd_status {
    KEA_CMD_STATUS_OK = 0,
    KEA_CMD_STATUS_ERR = 1,
    KEA_CMD_STATUS_MODEL_INVALID = 2,
    KEA_CMD_STATUS_STREAM_ERR = 3
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

#define KEA_MAX_RESP_PKT_LEN    (0xffff)

struct kea_cmd_all_models_req {
    struct kea_req_hdr hdr;
    uint8_t do_include_names;
} __attribute__((packed));

struct kea_rsp_model_meta {
    uint16_t model_id;
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

struct kea_cmd_get_model_schema_rsp {
    struct kea_rsp_hdr hdr;
    uint16_t model_id;
    unsigned char num_static_objs;
    unsigned char num_attrs;
    struct kea_cmd_model_attr attrs[0];
} __attribute__((packed));

struct kea_cmd_get_model_schema_req {
    struct kea_req_hdr hdr;
    uint16_t model_id;
    uint8_t do_include_names;
} __attribute__((packed));

bool kea_req_get_all_models(
    struct kea_stream *stream, bool do_include_names, unsigned char trans_id);

unsigned kea_cmd_get_all_models_decode_rsp(
    const void *rsp_buf, unsigned resp_len, struct kea_rsp_model_meta **models_info, unsigned max_model_array_size);

bool kea_req_get_model_schema(
    struct kea_stream *stream, uint16_t model_id, bool do_include_names, unsigned trans_id);

int kea_cmd_get_model_schema_decode_rsp(
    const void *rsp_buf, unsigned resp_len, struct kea_cmd_model_attr **attrs, unsigned max_attr_array_size);

#endif /* __KEA_CMDS_PUBLIC_IF_H */