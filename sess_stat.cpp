#include "sess_stat.h"

#include "os_util.hpp"
#include "toolkit.h"

#include <stdio.h>

sess_stat::sess_stat() {
	tick = nsp::os::clock_gettime();
}

sess_stat::~sess_stat() {
	tick = nsp::os::clock_gettime();
}

void sess_stat::increase_tx( uint64_t inc ) {
	sub_tx_ += inc;
	total_tx_ += inc;
}

void sess_stat::increase_rx( uint64_t inc ) {
	sub_rx_ += inc;
	total_rx_ += inc;

	// Rx 完成阶段累计IO完成量
	++io_counts_;
	++sub_io_counts_;
}

void sess_stat::print() {
    uint64_t current_tick = nsp::os::clock_gettime();
	if ( current_tick <= tick ) {
		return;
	}
    uint64_t escaped_tick = current_tick - tick;
    uint32_t escaped_seconds = (uint32_t) (escaped_tick / 10000000);
    tick = current_tick;
    char total_tx_unit[16], total_rx_unit[16];

    // session 至今总共的IO个数
    uint64_t io_counts = io_counts_;

    // session 至今总共的Tx数据量
    nsp::toolkit::posix_strcpy(total_tx_unit, cchof(total_tx_unit), "MB");
	uint64_t total_tx_u = total_tx_;
    double total_tx = (double) total_tx_u / 1024 / 1024;
    if (total_tx > 1024) {
        total_tx /= 1024;
        nsp::toolkit::posix_strcpy(total_tx_unit, cchof(total_tx_unit), "GB");
        if (total_tx > 1024) {
            total_tx /= 1024;
            nsp::toolkit::posix_strcpy(total_tx_unit, cchof(total_tx_unit), "TB");
        }
    }

    // session 至今总共的Rx数据量
    nsp::toolkit::posix_strcpy(total_rx_unit, cchof(total_rx_unit), "MB");
	uint64_t total_rx_u = total_rx_;
    double total_rx = (double) total_rx_u / 1024 / 1024;
    if (total_rx > 1024) {
        total_rx /= 1024;
        nsp::toolkit::posix_strcpy(total_rx_unit, cchof(total_rx_unit), "GB");
        if (total_rx > 1024) {
            total_rx /= 1024;
            nsp::toolkit::posix_strcpy(total_rx_unit, cchof(total_rx_unit), "TB");
        }
    }

    // 检查时间区间内的IOPS
    uint32_t iops = (uint32_t) sub_io_counts_;
    if (escaped_seconds > 1) {
        iops /= escaped_seconds;
    }

    // 检查时间区间内的Tx速度
    char tx_speed[128];
    uint64_t tx_bps_u =  sub_tx_;
    tx_bps_u *= 8;
    double tx_bps = (double) tx_bps_u;
    if (escaped_seconds > 1) {
        tx_bps /= escaped_seconds;
    }
    posix__sprintf(tx_speed, cchof(tx_speed), "%.1f bps", tx_bps);
    if (tx_bps / 1024 > 1) {
		tx_bps /= 1024;
        posix__sprintf(tx_speed, cchof(tx_speed), "%.1f Kbps", tx_bps);
    }
    if (tx_bps / 1024 > 1) {
        tx_bps /= 1024;
        posix__sprintf(tx_speed, cchof(tx_speed), "%.1f Mbps", tx_bps);
    }

    // 检查时间区间内的Rx速度
    char rx_speed[128];
    uint64_t rx_bps_u = sub_rx_;
    rx_bps_u *= 8;
    double rx_bps = (double) rx_bps_u;
    if (escaped_seconds > 1) {
        rx_bps /= escaped_seconds;
    }
    posix__sprintf(rx_speed, cchof(rx_speed), "%.1f bps", rx_bps);
    if (rx_bps / 1024 > 1) {
		rx_bps /= 1024;
        posix__sprintf(rx_speed, cchof(rx_speed), "%.1f Kbps",rx_bps );
    }
    if (rx_bps / 1024 > 1) {
		rx_bps /= 1024;
        posix__sprintf(rx_speed, cchof(rx_speed), "%.1f Mbps", rx_bps);
    }

#if __x86_64__
    printf("\t%lu\t\t%.3f(%s)\t%.3f(%s)\t%u\t%s\t%s\n", io_counts, total_tx, total_tx_unit, total_rx, total_rx_unit, iops, tx_speed, rx_speed);
#else
#if _WIN32
	printf("\t%I64u\t\t%.3f(%s)\t%.3f(%s)\t%u\t%s\t%s\n", io_counts, total_tx, total_tx_unit, total_rx, total_rx_unit,  iops, tx_speed, rx_speed);
#else
    printf("\t%llu\t\t%.3f(%s)\t%.3f(%s)\t%u\t%s\t%s\n", io_counts, total_tx, total_tx_unit, total_rx, total_rx_unit, iops, tx_speed, rx_speed);
#endif
#endif


    // 完成打印及清空当前数据
    sub_io_counts_ = 0;
    sub_rx_ = 0;
    sub_tx_ = 0;
}