#include "args.h"

#include "logger.h"
#include "nis.h"

extern int srv_startup();
extern int cli_startup();

void STDCALL ecr(const char *host_event, const char *reserved, int rescb)
{
    log__save("nshost", kLogLevel_Info, kLogTarget_Filesystem, "%s", host_event );
}

int main(int argc, char **argv)
{
    int type;
    int retval;

    if (arg_check_input(argc, argv) < 0) {
        return 1;
    }

    log_init();
    nis_checr(&ecr);
    tcp_init();

    type = arg_gettype();

    retval = -1;
    do {
        if (SESS_TYPE_SERVER == type) {
            if (srv_startup() < 0) {
                break;
            }
        } else {
            if (cli_startup() < 0) {
                break;
            }
        }

        retval = 0;
    }while(0);


    log__flush();
    return retval;
}
