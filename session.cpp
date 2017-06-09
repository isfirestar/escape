#include "session.h"
#include "os_util.hpp"
#include "toolkit.h"
#include "args.h"

const int proto_escape_task::length() const {
    return request_id_.length() + contex_.length();
}

unsigned char *proto_escape_task::serialize(unsigned char *bytes) const {
    unsigned char *pos = bytes;
    pos = request_id_.serialize(pos);
    pos = contex_.serialize(pos);
    return pos;
}

const unsigned char *proto_escape_task::build(const unsigned char *bytes, int &cb) {
    const unsigned char *pos = bytes;
    pos = request_id_.build(pos, cb);
    pos = contex_.build(pos, cb);
    return pos;
}

     ///////////////////////////////////////////////////// session //////////////////////////////////////////////////////
session::session() : base_session() {
    type_ = gettype();
    if (set_pktsize() < 0){
        throw 0x100;
    }
    session_check_tick_ = session_begin_tick_ = nsp::os::clock_gettime();
}

session::session(HTCPLINK lnk) : base_session(lnk) {
    type_ = gettype();
    if (set_pktsize() < 0){
        throw 0x100;
    }
    session_check_tick_ = session_begin_tick_ = nsp::os::clock_gettime();
}

session::~session() {
}

void session::on_recvdata(const std::string &data)  {
    // 累积接收字节量
    total_rx_ += data.size();
    sub_rx_ += data.size();
    // 累积IO完成量
    ++io_counts_;
    ++sub_io_counts_;
    
    uint32_t pktid = *((uint32_t *)data.c_str());
    this->write(type_ == 0 ? pktid : 0);
}

extern void end_client();

void session::on_disconnected(const HTCPLINK previous)  {
	if ( 1 == type_ ) {
		// 客户端链接断开， 直接找主线程， 通知退出
		end_client();
	}
	//printf("session disconnected.\n");
}

int session::set_pktsize(){    
    size_  = getpkgsize();
    char *p = nullptr;
    
    try {
        p = new char[size_];
    }catch(...){
        return -1;
    }
    
    packet_.contex_.assign(p, size_);
    delete []p;
    return 0;
}

int session::write(uint32_t pktid){
    if ( 0 == pktid ){
        pktid = ++ato_request_id_;
    }
    
    packet_.request_id_ = pktid;
    
    // 累计发送字节量
    sub_tx_ += size_;
    total_tx_ += size_;
    
    tick_psend_ = nsp::os::clock_gettime();
    return this->psend(&packet_);
}

void session::print(){
    
    uint64_t current_tick = nsp::os::clock_gettime();
    uint64_t escaped_tick =  current_tick - session_check_tick_;
    uint32_t escaped_seconds = escaped_tick / 10000000;
    session_check_tick_ = current_tick;
    
    // session 至今总共的IO个数
    uint64_t io_counts = io_counts_;
    // session 至今总共的Tx数据量
    double total_tx = (double)total_tx_ / 1024 / 1024;
    // session 至今总共的Rx数据量
    double total_rx = (double)total_rx_ / 1024 / 1024;
    // 检查时间区间内的IOPS
    uint32_t iops = sub_io_counts_;
    if (escaped_seconds > 0){
        iops /= escaped_seconds;
    }
    // 检查时间区间内的Tx速度
    char tx_speed[128];
    double tx_bps = sub_tx_ * 8;
    if (escaped_seconds > 0){
        tx_bps /= escaped_seconds;
    }
    posix__sprintf(tx_speed, cchof(tx_speed), "%u bps", tx_bps);
    if (tx_bps / 1024 > 1){
       posix__sprintf(tx_speed, cchof(tx_speed), "%.1f Kbps", tx_bps / 1024);
        tx_bps /= 1024;
    }
    if (tx_bps / 1024 > 1){
        posix__sprintf(tx_speed, cchof(tx_speed), "%.1f Mbps", tx_bps / 1024);
        tx_bps /= 1024;
    }
    // 检查时间区间内的Rx速度
    char rx_speed[128];
    double rx_bps = sub_rx_ * 8;
    if (escaped_seconds > 0){
        rx_bps /= escaped_seconds;
    }
    posix__sprintf(rx_speed, cchof(rx_speed), "%u bps", rx_bps);
    if (rx_bps / 1024 > 1){
        posix__sprintf(rx_speed, cchof(rx_speed), "%.1f Kbps", rx_bps / 1024);
        rx_bps /= 1024;
    }
    if (rx_bps / 1024 > 1){
        posix__sprintf(rx_speed, cchof(rx_speed), "%.1f Mbps", rx_bps / 1024);
        rx_bps /= 1024;
    }
    printf("\t%llu\t\t%.2f(MB)\t%.2f(MB)\t%u\t%s\t%s\n", io_counts, total_tx, total_rx, iops, tx_speed, rx_speed);
    
    // 完成打印及清空当前数据
    sub_io_counts_ = 0;
    sub_rx_ = 0;
    sub_tx_ = 0;
}