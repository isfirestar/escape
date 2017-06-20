#if !defined SESS_STAT_H
#define SESS_STAT_H

#include <atomic>
#include <cstdint>

struct sess_stat {
    // （不受查询循环影响的）总量统计
    std::atomic<uint64_t> io_counts_ {0} ; // 从开始到当前总共发生的IO次数
    std::atomic<uint64_t> total_rx_ {0};     // 从开始到当前总共接收的数据字节数
    std::atomic<uint64_t> total_tx_ {0};     // 从开始到当前总共发送的数据字节数
    
    // 单词查询区间的统计
    std::atomic<uint64_t> sub_io_counts_{0};
    std::atomic<uint64_t> sub_rx_ {0};
    std::atomic<uint64_t> sub_tx_ {0};
};

#endif // !SESS_STAT_H