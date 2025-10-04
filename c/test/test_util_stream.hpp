#pragma once

#include <string>
#include <vector>
#include <string>
#include <queue>

extern "C" {
    #include "kea/kea.h"
}

#define MAX_RECV_SIZE (256U)

class TestStream {
public:
    TestStream(unsigned kea_internal_buf_size);
    ~TestStream();

    unsigned getBufSize() const { return m_kea_internal_buf_size; }
    char *getBuf() const { return m_kea_internal_buf; }
    struct kea_stream *getCStream() { return &m_stream; }

    int getResp(char *buf, int buf_size);

    struct kea_stream m_stream;
    
    char *m_kea_internal_buf;
    unsigned m_kea_internal_buf_size;

    struct out_pkt {
        unsigned buf_len;
        unsigned offset;
        char *buf;
    };

    std::queue<struct out_pkt*> m_out_packets;
    static unsigned s_stream_num;

    std::string m_name;
};

class TestStreamPair {
public:
    TestStreamPair(unsigned client_buf_size, unsigned serv_buf_size);
    ~TestStreamPair() = default;

    class TestStream *getClientStream() { return &m_client; }
    class TestStream *getServerStream() { return &m_kea_server; }

    int getClientRsp(char *buf, int buf_size) { return m_client.getResp(buf, buf_size); };

    struct stream_priv_data {
        class TestStream *read_stream;
        class TestStream *self;
    };

    class TestStream m_kea_server;
    class TestStream m_client;

    struct stream_priv_data m_serv_data;
    struct stream_priv_data m_client_data;
};