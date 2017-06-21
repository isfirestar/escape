#include "icom/compiler.h"
#include <getopt.h>

#include "toolkit.h"
#include "endpoint.h"

#include "args.h"

static struct {
    int type; // @0: server @1:client
    char host[128]; // IP��ַ������
    uint16_t port; // �˿�
    uint32_t size_; // ����С
    uint32_t interval; // ��ӡ���
    char file[255]; // ʹ���ļ�
    int mode;
	int winsize;		// �������ڴ�С
} __startup_parameters;

enum ope_index {
    kOptIndex_GetHelp = 'h',
    kOptIndex_GetVersion = 'v',
    kOptIndex_SetHost = 'H', // @client: ����Ŀ��IP��ַ������ @server:���ؼ�����IP��ַ/����
    kOptIndex_SetPort = 'P', // @client: ����Ŀ��Ķ˿� @server:���ؼ����Ķ˿�
    kOptIndex_Server = 'S', // �Է�����������
    kOptIndex_Client = 'C', // �Կͻ����������
    kOptIndex_Size = 's', // @client: �������С @server:Ӧ�����С, ���ָ���� @u����@d, ��ò�����ʾ�ļ�����ĵ�Ƭ��С
    kOptIndex_DisplayInterval = 'i', // ��ӡ������뵥λ
    kOptIndex_UploadFile = 'u', // ִ��һ���ϴ��ļ�����
    kOptIndex_DownloadFile = 'd', // ִ��һ�������ļ�����
	kOptIndex_WinSize = 'n',	// winsize
	kOptIndex_WindowSize = 'w',  // winsize
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
    {"upload", required_argument, NULL, kOptIndex_UploadFile },
    {"download", required_argument, NULL, kOptIndex_DownloadFile},
	{"winsize", required_argument, NULL, kOptIndex_WinSize},
	{"window-size", required_argument, NULL, kOptIndex_WindowSize},
    {NULL, 0, NULL, 0}
};

void display_usage() {
    static const char *usage_context =
            "usage escape - nshost escape timing counter\n"
            "SYNOPSIS\n"
            "\t[-h|--help]\tdisplay usage context and help informations\n"
            "\t[-v|--version]\tdisplay versions of executable archive\n"
            "\t[-S|--server]\tthis session run as a server.\n"
            "\t[-C|--client]\tthis session run as a client.this option is default.\n"
            "\t[-H|--host]\ttarget server IPv4 address or domain when @C or local bind adpater when @S, \"0.0.0.0\" by default\n"
            "\t[-P|--port]\ttarget server TCP port when @C or local listen TCP port when @S, 10256 by default\n"
            "\t[-s|--size]\trequest packet size in bytes when @C or response packet size in bytes when @S 1024 by default\n"
			"\t\t\twhen using @u or @d, @s means pre-block bytes in file mode\n"
            "\t[-i|--interval]\tinterval in seconds between two displays.1 sec by default. \n"
            "\t[-u|--upload]\tfile mode of upload from @args on local to server application startup directory. only when @C.\n"
            "\t[-d|--download]\tfile mode of download from @args on server to current directory.only @C.\n"
			"\t[-n|-w|--winsize|--window-size]\tpacket transfer window size of this connection. 1 by default. only @C and conflict with file mode.\n"
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
    
    __startup_parameters.type = SESS_TYPE_CLIENT;
    __startup_parameters.size_ = 1024;
    nsp::toolkit::posix_strcpy(__startup_parameters.host, cchof(__startup_parameters.host), "0.0.0.0");
    __startup_parameters.port = 10256;
    __startup_parameters.interval = 1000;
    __startup_parameters.mode = CS_MODE_ESCAPE_TASK;
    memset(__startup_parameters.file, 0, sizeof(__startup_parameters.file));
	__startup_parameters.winsize = 1;

    nsp::toolkit::posix_strcpy(shortopts, cchof(shortopts), "SCvhH:P:s:i:u:d:n:w:");
    opt = getopt_long(argc, argv, shortopts, long_options, &opt_index);
    while (opt != -1) {
        switch (opt) {
            case 'S':
                __startup_parameters.type = SESS_TYPE_SERVER;
                break;
            case 'C':
                __startup_parameters.type = SESS_TYPE_CLIENT;
                break;
            case 'v':
                display_author_information();
                return -1;
            case 'h':
                display_usage();
                return -1;
            case 'H':
                nsp::toolkit::posix_strcpy(__startup_parameters.host, cchof(__startup_parameters.host), optarg);
                break;
            case 'P':
                __startup_parameters.port = (uint16_t) strtoul(optarg, NULL, 10);
                break;
            case 's':
                __startup_parameters.size_ = strtoul(optarg, NULL, 10);
                break;
            case 'i':
                __startup_parameters.interval = strtoul(optarg, NULL, 10) * 1000;
                break;
            case 'u':
                __startup_parameters.mode = CS_MODE_FILE_UPLOAD;
                nsp::toolkit::posix_strcpy(__startup_parameters.file, sizeof(__startup_parameters.file), optarg);
                break;
            case 'd':
                __startup_parameters.mode = CS_MODE_FILE_DOWNLOAD;
                nsp::toolkit::posix_strcpy(__startup_parameters.file, sizeof(__startup_parameters.file), optarg);
                break;
			case 'w':
			case 'n':
				__startup_parameters.winsize = strtoul(optarg, NULL, 10);
				break;
            case '?':
                printf("?\n");
            case 0:
                printf("0\n");
            default:
                display_usage();
                return -1;
        }
        opt = getopt_long(argc, argv, shortopts, long_options, &opt_index);
    }

    if ((__startup_parameters.mode > CS_MODE_MUST_BE_CLIENT) && __startup_parameters.type != SESS_TYPE_CLIENT){
        display_usage();
        return -1;
    }
	if ( __startup_parameters.type == SESS_TYPE_CLIENT && 0 == nsp::toolkit::posix_strcasecmp( __startup_parameters.host, "0.0.0.0" ) ) {
		display_usage();
		return -1;
	}
	if ( __startup_parameters.winsize > 1 && __startup_parameters.type == SESS_TYPE_SERVER ) {
		display_usage();
		return -1;
	}
    return retval;
}

int buildep(nsp::tcpip::endpoint &ep){
    return nsp::tcpip::endpoint::build(__startup_parameters.host, __startup_parameters.port, ep);
}

int gettype(){
    return __startup_parameters.type;
}

int getpkgsize(){
    return __startup_parameters.size_;
}

int getinterval(){
    return __startup_parameters.interval;
}

extern int getmode() {
    return __startup_parameters.mode;
}

const char *getfile( int *len ) {
	if ( len ) {
		*len = strlen( __startup_parameters.file );
	}
	return &__startup_parameters.file[0];
}

int getwinsize() {
	return __startup_parameters.winsize;
}