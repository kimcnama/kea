#ifndef __KEA_MODEL_PUBLIC_IF_H
#define __KEA_MODEL_PUBLIC_IF_H

#include <stdbool.h>
#include <stdint.h>

#define MODEL_HASH_SZ (8)

enum kea_data_types {
    KEA_TYPE_UNDEFINED = 0,
    KEA_TYPE_INT8,
    KEA_TYPE_UINT8,
    KEA_TYPE_INT16,
    KEA_TYPE_UINT16,
    KEA_TYPE_INT32,
    KEA_TYPE_UINT32,
    KEA_TYPE_INT64,
    KEA_TYPE_UINT64,
    KEA_TYPE_FLOAT,
    KEA_TYPE_DOUBLE,
    KEA_TYPE_STRING,
    KEA_TYPE_STRUCT,
    KEA_TYPE_BINARY
};

/* Lookup array mapping enum kea_data_types to size in bytes.
 * For types with variable size (STRING, STRUCT, BINARY), use 0.
 */
static const unsigned char kea_data_type_sizes[] = {
    0,                          /* KEA_TYPE_UNDEFINED */
    sizeof(int8_t),             /* KEA_TYPE_INT8 */
    sizeof(uint8_t),            /* KEA_TYPE_UINT8 */
    sizeof(int16_t),            /* KEA_TYPE_INT16 */
    sizeof(uint16_t),           /* KEA_TYPE_UINT16 */
    sizeof(int32_t),            /* KEA_TYPE_INT32 */
    sizeof(uint32_t),           /* KEA_TYPE_UINT32 */
    sizeof(int64_t),            /* KEA_TYPE_INT64 */
    sizeof(uint64_t),           /* KEA_TYPE_UINT64 */
    sizeof(float),              /* KEA_TYPE_FLOAT */
    sizeof(double),             /* KEA_TYPE_DOUBLE */
    0,                          /* KEA_TYPE_STRING (variable size) */
    0,                          /* KEA_TYPE_STRUCT (variable size) */
    0                           /* KEA_TYPE_BINARY (variable size) */
};

enum kea_data_collection_types {
    KEA_COLLECTION_TYPE_UNDEFINED = 0,
    KEA_COLLECTION_TYPE_SCALAR,
    KEA_COLLECTION_TYPE_ARRAY_STATIC,
    KEA_COLLECTION_TYPE_ARRAY_DYNAMIC,
    KEA_COLLECTION_TYPE_CUSTOM
};

struct kea_obj_access_hdr;

struct kea_model_hdr {
    uint16_t model_id;
    const char *name;
    unsigned char hash[MODEL_HASH_SZ];
    unsigned char num_static_objs;
    unsigned char num_dyn_objs;
    struct kea_obj_access_hdr *dyn_objs;
    struct kea_obj_access_hdr **objs;
};

struct kea_obj_access_hdr {
    unsigned char obj_id;
    unsigned init_status: 1;
    unsigned collection_type: 3;
    unsigned type: 4;
    char *name;
    struct kea_obj_access_hdr *next;
    void *self;
};

struct kea_obj_static_array {
    struct kea_obj_access_hdr hdr;
    unsigned long num_elems;
    unsigned char elem_size;
};

struct kea_obj_custom {
    struct kea_obj_access_hdr hdr;
    
    /* Init method */
    int (*init)(void **self);
    /* Total size in bytes */
    int (*size)(void *self);
    
    /* Get bytes */
    int (*get_bytes)(void *self, unsigned bytes_offset, unsigned num_bytes, char *buf);
    /* Set bytes */
    int (*set_bytes)(void *self, unsigned bytes_offset, unsigned num_bytes, const char *buf);
};

struct kea_obj_dynamic_array {
    struct kea_obj_access_hdr hdr;
    
    /* Init method */
    int (*init)(void **self);
    /* Total size in bytes */
    int (*size)(void *self);
    /* Number of elements */
    int (*shape)(void *self, unsigned long **shape, unsigned char *shape_len);
    
    /* Get element */
    int (*get_elem)(void *self, unsigned long *idx, unsigned char idx_len, void *buf, unsigned buf_size);
    /* Set element */
    int (*set_elem)(void *self, unsigned long *idx, unsigned char idx_len, const void *buf, unsigned buf_size);
    /* Delete element */
    int (*delete_elem)(void *self, unsigned long *idx, unsigned char idx_len);
    
    /* Get bytes */
    int (*get_bytes)(void *self, unsigned bytes_offset, unsigned num_bytes, char *buf);
    /* Set bytes */
    int (*set_bytes)(void *self, unsigned bytes_offset, unsigned num_bytes, const char *buf);
    /* Delete bytes */
    int (*delete_bytes)(void *self, unsigned bytes_offset, unsigned num_bytes);
};

struct kea_model_iter {
    unsigned cur_idx;
};

int kea_model_get_next_free_model_id(void);
int kea_model_register(struct kea_model_hdr *model, bool use_first_free_id);
unsigned char kea_get_num_registered_models(void);
void kea_clear_all_models(void);

#endif /* __KEA_MODEL_PUBLIC_IF_H */
