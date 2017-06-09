#include "icom/compiler.h"
#include <getopt.h>

#include "toolkit.h"
#include "endpoint.h"

static struct {
    int type_; // @0: server @1:client
    char host_[128]; // IP地址或域名
    uint16_t port_; // 端口
    uint32_t size_; // 包大小
    uint32_t interval_; // 打印间隔
} __startup_parameters;

enum ope_index {
    kOptIndex_GetHelp = 'h',
    kOptIndex_GetVersion = 'v',
    kOptIndex_SetHost = 'H', // @client: 链接目标IP地址获域名 @server:本地监听的IP地址/网卡
    kOptIndex_SetPort = 'P', // @client: 连接目标的端口 @server:本地监听的端口
    kOptIndex_Server = 'S', // 以服务端身份运行
    kOptIndex_Client = 'C', // 以客户端身份运行
    kOptIndex_Size = 's', // @client: 请求包大小 @server:应答包大小, 均为字节单位
    kOptIndex_DisplayInterval = 'i' // 打印间隔，秒单位
};

static const struct option long_options[] = {
    {"help", no_argument, NULL, kOptIndex_GetHelp},
    {"host", required_argument, NULL, kOptIndex_SetHost},
    {"version", no_argument, NULL, kOptIndex_GetVersion},
    {"port", required_argument, NULL, kOptIndex_SetPort},
    {"server", no_argument, NULL, kOptIndex_Server},
    {"client", no_argument, NULL, kOptIndex_Client},
    {"size", required_argument, NULL, kOptIndex_Size},
    {"interval", required_argument, NULL, kOptIndex_DisplayInterval},
    {NULL, 0, NULL, 0}
};

void dispay_usage() {
    static const char *usage_context =
            "usage escape - nshost escape timing counter\n"
            "SYNOPSIS\n"
            "\t[-h|--help]\tdisplay usage context and help informations\n"
            "\t[-v|--version]\tdisplay versions of executable archive\n"
            "\t[-S|--server]\tthis session run as a server.\n"
            "\t[-C|--client]\tthis session run as a client.\n"
            "\t[-H|--host]\ttarget server IPv4 address or domian when @C or local bind adpater when @S 0.0.0.0 by default\n"
            "\t[-P|--port]\ttarget server TCP port when @C or local linsten TCP port when @S 10256 by default\n"
            "\t[-s|--size]\trequest packet size in bytes when @C or response packet size in bytes when @S 1024 by default\n"
            "\t[-i|--interval]\tinterval in seconds between two displays.1 sec by default. \n"
            ;

    printf(usage_context);
}

static
void display_author_information() {
    static const char *author_context =
            "nshost escape 1,1,0,0\n"
            "Copyright (C) 2017 neo.anderson\n"
            "For bug reporting instructions, please see:\n"
            "<http://www.nshost.com.cn/>.\n"
            "For help, type \"help\".\n"
            ;
    printf(author_context);
}

int check_args(int argc, char **argv) {
    int opt_index;
    int opt;
    int retval = 0;
    char shortopts[128];
    
    __startup_parameters.type_ = -1;
    __startup_parameters.size_ = 1024;
    nsp::toolkit::posix_strcpy(__startup_parameters.host_, cchof(__startup_parameters.host_), "0.0.0.0");
    __startup_parameters.port_ = 10256;
    __startup_parameters.interval_ = 1000;

    nsp::toolkit::posix_strcpy(shortopts, cchof(shortopts), "SCvhH:P:s:i:");
    opt = getopt_long(argc, argv, shortopts, long_options, &opt_index);
    while (opt != -1) {
        switch (opt) {
            case 'S':
                __startup_parameters.type_ = 0;
                break;
            case 'C':
                __startup_parameters.type_ = 1;
                break;
            case 'v':
                display_author_information();
                return -1;
            case 'h':
                dispay_usage();
                return -1;
            case 'H':
                nsp::toolkit::posix_strcpy(__startup_parameters.host_, cchof(__startup_parameters.host_), optarg);
                break;
            case 'P':
                __startup_parameters.port_ = (uint16_t) strtoul(optarg, NULL, 10);
                break;
            case 's':
                __startup_parameters.size_ = strtoul(optarg, NULL, 10);
                break;
            case 'i':
                __startup_parameters.interval_ = strtoul(optarg, NULL, 10) * 1000;
                break;
            case '?':
                printf("?\n");
                break;
            case 0:
                printf("0\n");
                break;
            default:
                retval = -1;
                dispay_usage();
                return -1;
        }
        opt = getopt_long(argc, argv, shortopts, long_options, &opt_index);
    }

    return retval;
}

int buildep(nsp::tcpip::endpoint &ep){
    return nsp::tcpip::endpoint::build(__startup_parameters.host_, __startup_parameters.port_, ep);
}

int gettype(){
    return __startup_parameters.type_;
}

int getpkgsize(){
    return __startup_parameters.size_;
}

int getinterval(){
    return __startup_parameters.interval_;
}