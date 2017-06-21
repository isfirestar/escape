#if !defined SESS_STAT_H
#define SESS_STAT_H

#include <atomic>
#include <cstdint>

class sess_stat {
    // �����ܲ�ѯѭ��Ӱ��ģ�����ͳ��
    std::atomic<uint64_t> io_counts_ {0} ; // �ӿ�ʼ����ǰ�ܹ�������IO����
    std::atomic<uint64_t> total_rx_ {0};     // �ӿ�ʼ����ǰ�ܹ����յ������ֽ���
    std::atomic<uint64_t> total_tx_ {0};     // �ӿ�ʼ����ǰ�ܹ����͵������ֽ���
    
    // ���ʲ�ѯ�����ͳ��
    std::atomic<uint64_t> sub_io_counts_{0};
    std::atomic<uint64_t> sub_rx_ {0};
    std::atomic<uint64_t> sub_tx_ {0};

	uint64_t tick = 0; // session ���ʱ���

public:
	sess_stat();
	~sess_stat();

	void increase_tx(uint64_t inc);
	void increase_rx(uint64_t inc);
	void print();
};

#endif // !SESS_STAT_H