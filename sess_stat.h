#if !defined SESS_STAT_H
#define SESS_STAT_H

#include <atomic>
#include <cstdint>

struct sess_stat {
    // �����ܲ�ѯѭ��Ӱ��ģ�����ͳ��
    std::atomic<uint64_t> io_counts_ {0} ; // �ӿ�ʼ����ǰ�ܹ�������IO����
    std::atomic<uint64_t> total_rx_ {0};     // �ӿ�ʼ����ǰ�ܹ����յ������ֽ���
    std::atomic<uint64_t> total_tx_ {0};     // �ӿ�ʼ����ǰ�ܹ����͵������ֽ���
    
    // ���ʲ�ѯ�����ͳ��
    std::atomic<uint64_t> sub_io_counts_{0};
    std::atomic<uint64_t> sub_rx_ {0};
    std::atomic<uint64_t> sub_tx_ {0};
};

#endif // !SESS_STAT_H