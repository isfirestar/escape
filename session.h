#if !defined SESSION_H
#define SESSION_H

#include <atomic>
#include <map>

#include "application_network_framwork.hpp"
#include "old.hpp"
#include "serialize.hpp"

struct proto_escape_task : public nsp::proto::proto_interface {
    virtual const int length() const override final;
    virtual unsigned char *serialize(unsigned char *bytes) const override final ;
    virtual const unsigned char *build(const unsigned char *bytes, int &cb) override final;
    
    nsp::proto::proto_crt_t<uint32_t> request_id_;
    nsp::proto::proto_string_t<char>  contex_;
};

typedef nsp::tcpip::tcp_application_client<nsp::proto::nspdef::protocol> base_session;

class session : public base_session {
    int size_ = 1024;
    struct proto_escape_task packet_;
    std::atomic<uint32_t> ato_request_id_ { 0 };
    int type_;
    
    uint64_t session_begin_tick_;           // session 构造时间点
    uint64_t session_check_tick_ = 0;          // session 检查时间点
    
    // （不受查询循环影响的）总量统计
    std::atomic<uint64_t> io_counts_ {0} ; // 从开始到当前总共发生的IO次数
    std::atomic<uint64_t> total_rx_ {0};     // 从开始到当前总共接收的数据字节数
    std::atomic<uint64_t> total_tx_ {0};     // 从开始到当前总共发送的数据字节数
    
    // 单词查询区间的统计
    std::atomic<uint64_t> sub_io_counts_{0};
    std::atomic<uint64_t> sub_rx_ {0};
    std::atomic<uint64_t> sub_tx_ {0};
    std::atomic<uint64_t> sub_avg_delay {0}; // 单位区间内的平均延迟
    
    // 发送请求包的时间
    std::atomic<uint64_t> tick_psend_ {0};
    
    int set_pktsize();

public:
    session();
    session(HTCPLINK lnk);
    ~session();
    virtual void on_recvdata(const std::string &data) override;
    virtual void on_disconnected(const HTCPLINK previous) override;
    
    int write(uint32_t pktid = 0);
    void print();
};

#endif