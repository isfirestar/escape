#include "args.h"

#include <getopt.h>

#include "posix_string.h"

static struct {
    int type; /* @0: server @1:client */
    struct misc_endpoint endpoint;  /* endpoint specified by startup argument */
    int txn;    /* number of bytes in Tx(request) packet. */
    int rxn;    /* number of bytes in Rx(response) packet */
    int interval; /* time interval in milliseconds during every two statistic print */
	int window;		/* window size for every TCP link */
} __startup_parameters;

enum ope_index {
    kOptIndex_GetHelp = 'h',        /* 'h' is the abbreviation of 'help' */
    kOptIndex_GetVersion = 'v',     /* 'v' is the abbreviation of 'version' */
    kOptIndex_Server = 's',         /* 's' is the abbreviation of 'server' */
    kOptIndex_Client = 'c',         /* 'c' is the abbreviation of 'client' */
    kOptIndex_TxSize = 't',         /* 't' is the abbreviation of 'transmit' */
    kOptIndex_RxSize = 'r',         /* 'r' is the abbreviation of 'response' */
    kOptIndex_Interval = 'i',       /* 'i' is the abbreviation of 'interval' */
	kOptIndex_WindowSize = 'w',     /* 'w' is the abbreviation of 'window' */
};

static const struct option long_options[] = {
    {"help", no_argument, NULL, kOptIndex_GetHelp},
    {"version", no_argument, NULL, kOptIndex_GetVersion},
    {"server", optional_argument, NULL, kOptIndex_Server},
    {"client", required_argument, NULL, kOptIndex_Client},
    {"transmit", required_argument, NULL, kOptIndex_TxSize},
    {"acknowledge", required_argument, NULL, kOptIndex_RxSize},
    {"interval", required_argument, NULL, kOptIndex_Interval},
	{"window", required_argument, NULL, kOptIndex_WindowSize},
    {NULL, 0, NULL, 0}
};

static void arg_display_usage()
{
    static const char *usage_context =
            "usage escape - a nshost test program\n"
            "SYNOPSIS\n"
            "\t[-h|--help]\tdisplay usage context and help informations\n"
            "\t[-v|--version]\tdisplay versions of executable archive\n"
            "\t[-s|--server]\tindicate that escape program running under server model,this option is default.\n"
            "\t\t\tserver model optional specify a endpoint indicate the escape service listen on.\n"
            "\t\t\t \"0.0.0.0:10256\" by default\n"
            "\t[-c|--client]\tindicate that escape prgoram running under client model.\n"
            "\t\t\tclient model acquire a explicit endpoint indicate target host which escape program connect to \n"
            "\t[-t|--transmit]\tusing this option only in client model.\n"
            "\t\t\ttell the escape program how many bytes each request packet send\n"
            "\t\t\tdefault transmit size is 1428 Bytes.\n"
            "\t[-r|--response]\tusing this option only in server model.\n"
            "\t\t\ttell the escape program how many bytes each response packet acknowledge\n"
            "\t\t\tdefault response size are equivalent to request.\n"
            "\t[-i|--interval]\tinterval in milliseconds between two statistic point, 1 second by default. \n"
			"\t[-w|--window]\tpacket transfer window size of this connection.\n"
            ;

    printf("%s", usage_context);
}

static void arg_display_author_information()
{
    static const char *author_context =
            "nshost escape 2,1,0,0\n"
            "Copyright (C) 2017 neo.anderson\n"
            "For bug reporting instructions, please see:\n"
            "<http://www.nshost.com.cn/>.\n"
            "For help, type \"help\".\n"
            ;
    printf("%s", author_context);
}

int arg_check_input(int argc, char **argv)
{
    int opt_index;
    int opt;
    char shortopts[32];

    /*  MAX_TCP_UNIT - nsp-head(8) - packet-id(4) - timestamp(8) */
    static const int DEFAULT_ESCAPE_SIZE = 1428;

    __startup_parameters.type = SESS_TYPE_SERVER;
    misc_build_endpoint("0.0.0.0:10256", &__startup_parameters.endpoint);
    __startup_parameters.txn = DEFAULT_ESCAPE_SIZE;
    __startup_parameters.rxn = 0;
    __startup_parameters.interval = 1000;
	__startup_parameters.window = 1;

    posix__strcpy(shortopts, cchof(shortopts), "hvs,c:t:r:i:w:");
    opt = getopt_long(argc, argv, shortopts, long_options, &opt_index);
    while (opt != -1) {
        switch (opt) {
            case kOptIndex_Server:
                __startup_parameters.type = SESS_TYPE_SERVER;
                if(optarg) {
                    if (misc_build_endpoint(optarg, &__startup_parameters.endpoint) < 0) {
                        return -1;
                    }
                }
                break;
            case kOptIndex_Client:
                __startup_parameters.type = SESS_TYPE_CLIENT;
                assert(optarg);
                if (misc_build_endpoint(optarg, &__startup_parameters.endpoint) < 0) {
                    return -1;
                }
                break;
            case kOptIndex_TxSize:
                assert(optarg);
                __startup_parameters.txn = atoi(optarg);
                break;
            case kOptIndex_RxSize:
                assert(optarg);
                __startup_parameters.rxn = atoi(optarg);
                break;
            case kOptIndex_Interval:
                assert(optarg);
                __startup_parameters.interval = strtoul(optarg, NULL, 10) * 1000;
                break;
			case kOptIndex_WindowSize:
                assert(optarg);
				__startup_parameters.window = strtoul(optarg, NULL, 10);
				break;
            case kOptIndex_GetVersion:
                arg_display_author_information();
                return -1;
            case kOptIndex_GetHelp:
            case '?':
            case 0:
            default:
                arg_display_usage();
                return -1;
        }
        opt = getopt_long(argc, argv, shortopts, long_options, &opt_index);
    }

    return 0;
}

int arg_gettype()
{
    return __startup_parameters.type;
}


void arg_duplicate_client_argument(struct misc_endpoint *server, int *txn, int *interval, int *window)
{
    if (server) {
        misc_duplicate_endpoint(server, &__startup_parameters.endpoint);
    }

    if (txn) {
        *txn = __startup_parameters.txn;
    }

    if (interval) {
        *interval = __startup_parameters.interval;
    }

    if (window) {
        *window = __startup_parameters.window;
    }
}

void arg_duplicate_server_argument(struct misc_endpoint *server, int *rxn)
{
    if (server) {
        misc_duplicate_endpoint(server, &__startup_parameters.endpoint);
    }

    if (rxn) {
        *rxn = __startup_parameters.rxn;
    }
}
