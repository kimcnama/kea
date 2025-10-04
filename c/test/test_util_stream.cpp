#pragma once

#include <gtest/gtest.h>
#include "test_util_stream.hpp"

static int test_stream_read(struct kea_stream *stream, char *buf, unsigned buf_size)
{
    int bytes_read = 0;
    int len;
    
    TestStreamPair::stream_priv_data *priv = (TestStreamPair::stream_priv_data*)stream->priv_data;
    class TestStream *read_stream = priv->read_stream;

    while (read_stream->m_out_packets.size() && buf_size > 0) {
        TestStream::out_pkt *cur_pkt = read_stream->m_out_packets.front();
        len = std::min(buf_size, cur_pkt->buf_len - cur_pkt->offset);
        memcpy(&buf[bytes_read], &cur_pkt->buf[cur_pkt->offset], len);
        buf_size -= len;
        bytes_read += len;
        cur_pkt->offset += len;
        if (cur_pkt->offset >= cur_pkt->buf_len) {
            free(cur_pkt);
            read_stream->m_out_packets.pop();
        }
    }

    return bytes_read;
}

static int test_stream_write(struct kea_stream *stream, const char *buf, unsigned len)
{
    TestStreamPair::stream_priv_data *priv = (TestStreamPair::stream_priv_data*)stream->priv_data;
    class TestStream *self = priv ? priv->self : nullptr;
    if (self == nullptr)
        return -EINVAL; // or another error code indicating invalid argument
    
    TestStream::out_pkt *pkt = (TestStream::out_pkt*)malloc(sizeof(TestStream::out_pkt) + len);
    if (pkt == nullptr)
        return -ENOMEM;
    
    pkt->buf_len = len;
    pkt->offset = 0;
    pkt->buf = &((char*)pkt)[sizeof(TestStream::out_pkt)];
    
    memcpy(pkt->buf, buf, len);
    self->m_out_packets.push(pkt);
    return len;
}

unsigned TestStream::s_stream_num = 0;
TestStream::TestStream(unsigned kea_internal_buf_size) {

    if (kea_internal_buf_size > 0) {
        m_kea_internal_buf = (char*)malloc(kea_internal_buf_size);
        EXPECT_NE(m_kea_internal_buf, nullptr);
    } else {
        m_kea_internal_buf = nullptr;
    }
    m_kea_internal_buf_size = kea_internal_buf_size;

    m_name = "stream_" +  std::to_string(s_stream_num++);
    memcpy(m_stream.metadata, m_name.c_str(), std::min(sizeof(m_stream.metadata), m_name.size()));

    m_stream.read = test_stream_read;
    m_stream.write = test_stream_write;
}

TestStream::~TestStream() {
    free(m_kea_internal_buf);
    while (!m_out_packets.empty()) {
        free(m_out_packets.front());
        m_out_packets.pop();
    }
}

int TestStream::getResp(char *buf, int buf_size) {
    return test_stream_read(&m_stream, buf, buf_size);
}

TestStreamPair::TestStreamPair(unsigned client_buf_size, unsigned serv_buf_size) :
    m_client(client_buf_size),
    m_kea_server(serv_buf_size)
{
    m_serv_data.read_stream = &m_client;
    m_serv_data.self = &m_kea_server;

    m_client_data.read_stream = &m_kea_server;
    m_client_data.self = &m_client;

    m_client.m_stream.priv_data = &m_client_data;
    m_kea_server.m_stream.priv_data = &m_serv_data;
}
