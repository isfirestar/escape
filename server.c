#include "nis.h"
#include "args.h"
#include "tst.h"

#include "proto.h"

#include <unistd.h>

static unsigned char *rx_buffer = NULL;
static int rx_size = 0;

static void srv_receive_data(HTCPLINK link, const unsigned char *data, int len)
{
	if (len >= sizeof(struct escape_head) && rx_buffer && rx_size >= sizeof(struct escape_head)) {
		((struct escape_head *)rx_buffer)->seq = ((const struct escape_head *)data)->seq;
		((struct escape_head *)rx_buffer)->tx_timestamp = ((const struct escape_head *)data)->tx_timestamp;
		tcp_write(link, rx_buffer, rx_size, NULL);
	}
}

static void STDCALL srv_callback(const struct nis_event *event, const void *data)
{
	tcp_data_t *tcpdata = (tcp_data_t *)data;

	if (event->Event == EVT_TCP_ACCEPTED) {
		return;
	}

	if (EVT_RECEIVEDATA == event->Event) {
		srv_receive_data(event->Ln.Tcp.Link, tcpdata->e.Packet.Data, tcpdata->e.Packet.Size);
		return;
	}
}

int srv_startup()
{
	struct misc_endpoint endpoint;
	int rxn;
	HTCPLINK link;
	int attr;
	tst_t tst;

	arg_duplicate_server_argument(&endpoint, &rxn);

	link = tcp_create(&srv_callback, endpoint.host, endpoint.port);
	if (link < 0) {
		misc_alarm_error("fails create TCP link on target %s:%d", endpoint.host, endpoint.port );
		return -1;
	}

	attr = nis_cntl(link, NI_GETATTR);
	nis_cntl(link, NI_SETATTR, attr | LINKATTR_TCP_UPDATE_ACCEPT_CONTEXT);
	tst.parser_ = &tst_parser;
    tst.builder_ = &tst_builder;
    tst.cb_ = sizeof(head_t);
    nis_cntl(link, NI_SETTST, &tst);

    /* allocate the response packet */
    rx_size = sizeof(struct escape_head) + rxn;
    rx_buffer = (unsigned char *)malloc(rx_size);
    assert(rx_buffer);

	tcp_listen(link, 100);

	while (1) sleep(1);
}
