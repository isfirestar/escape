#include "args.h"

#include "logger.h"
#include "nis.h"

void STDCALL ecr(const char *host_event, const char *reserved, int rescb)
{
    misc_alarm_trace(host_event);
}

int main(int argc, char **argv)
{
    int type;

    if (arg_check_input(argc, argv) < 0) {
        return 1;
    }

    log_init();
    nis_checr(&ecr);
    tcp_init();

    type = arg_gettype();
    if (SESS_TYPE_SERVER == type) {
        srv_startup();
    } else {
        cli_startup();
    }

    log__flush();
    return 0;
}
