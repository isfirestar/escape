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
    
    uint64_t session_begin_tick_;           // session ����ʱ���
    uint64_t session_check_tick_ = 0;          // session ���ʱ���
    
    // �����ܲ�ѯѭ��Ӱ��ģ�����ͳ��
    std::atomic<uint64_t> io_counts_ {0} ; // �ӿ�ʼ����ǰ�ܹ�������IO����
    std::atomic<uint64_t> total_rx_ {0};     // �ӿ�ʼ����ǰ�ܹ����յ������ֽ���
    std::atomic<uint64_t> total_tx_ {0};     // �ӿ�ʼ����ǰ�ܹ����͵������ֽ���
    
    // ���ʲ�ѯ�����ͳ��
    std::atomic<uint64_t> sub_io_counts_{0};
    std::atomic<uint64_t> sub_rx_ {0};
    std::atomic<uint64_t> sub_tx_ {0};
    std::atomic<uint64_t> sub_avg_delay {0}; // ��λ�����ڵ�ƽ���ӳ�
    
    // �����������ʱ��
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