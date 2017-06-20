#include <string.h>
#include <memory>
#include "session.h"
#include "args.h"
#include "log.h"
#include "os_util.hpp"

#include <errno.h>

nsp::os::waitable_handle server_wait;

int begin_server() {

    nsp::tcpip::endpoint ep;
    if (buildep(ep) < 0) {
        nsperror << "unknown EP specified,";
        return -1;
    }

    try {
        std::shared_ptr<nsp::tcpip::tcp_application_service < session>> srv = std::make_shared<nsp::tcpip::tcp_application_service < session >> ();
        srv->begin(ep);
    } catch (...) {
        nsperror << "failed to create TCP service";
        return -1;
    }

    server_wait.wait(-1);
    return 0;
}

nsp::os::waitable_handle client_wait;

int begin_client() {
    nsp::tcpip::endpoint ep;
    if (buildep(ep) < 0) {
        nsperror << "unknown EP specified,";
        return -1;
    }

    std::shared_ptr<session> cli;

    try {
        cli = std::make_shared<session> ();
        cli->create();
        cli->connect(ep);
        cli->begin_client();
		if ( 0 != cli->waitfor_init_finish() ) {
			return -1;
		}
    } catch (...) {
        nsperror << "failed to create TCP service";
        return -1;
    }

    // 如果一切顺利， 则客户端就用主线程，周期性查询网络消耗
    printf("\tio(total)\tTx(total)\tRx(total)\tiops\tTx(speed)\tRx(speed)\n");

    while (client_wait.wait(getinterval()) > 0) {
        cli->print();
    }

    return 0;
}

void end_client() {
    client_wait.sig();
}

int main(int argc, char **argv) {

    if (check_args(argc, argv) < 0) {
        return 1;
    }

    int type = gettype();
    switch (type) {
        case 0:
            if (begin_server() < 0) {
                return 1;
            }
            break;
        case 1:
            if (begin_client() < 0) {
                return 1;
            }
            break;
        default:
            dispay_usage();
            return 1;
    }
    return 0;
}