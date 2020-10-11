#include "nis.h"
#include "args.h"
#include "tst.h"

#include "proto.h"

#include "posix_wait.h"
#include "posix_time.h"

struct stat_item
{
	int64_t io;
	int64_t rx;
	int64_t tx;
};

struct statistic
{
	struct stat_item total;
	struct stat_item range;
	int64_t rtt;
	unsigned char *tx_buffer;
	int tx_size;
	posix__waitable_handle_t *wait;
};
static struct statistic __statistic = { { 0, 0, 0 }, { 0, 0, 0 }, 0, NULL };

static void srv_receive_data(HTCPLINK link, const unsigned char *data, int len)
{
	int64_t timestamp;
	int64_t rtt;

	timestamp = posix__clock_monotonic();

	__atomic_add_fetch(&__statistic.total.io, 1, __ATOMIC_SEQ_CST);
	__atomic_add_fetch(&__statistic.range.io, 1, __ATOMIC_SEQ_CST);

	__atomic_add_fetch(&__statistic.total.rx, len, __ATOMIC_SEQ_CST);
	__atomic_add_fetch(&__statistic.range.rx, len, __ATOMIC_SEQ_CST);

	rtt = timestamp - ((const struct escape_head *)data)->tx_timestamp;
	if (0 == __atomic_load_n(&__statistic.rtt, __ATOMIC_SEQ_CST) || rtt > __atomic_load_n(&__statistic.rtt, __ATOMIC_SEQ_CST)) {
		__atomic_store_n(&__statistic.rtt, rtt, __ATOMIC_SEQ_CST);
	}

	((struct escape_head *)__statistic.tx_buffer)->seq = ((const struct escape_head *)data)->seq + 1;
    ((struct escape_head *)__statistic.tx_buffer)->tx_timestamp = posix__clock_monotonic();
	tcp_write(link, __statistic.tx_buffer, len, NULL);

	__atomic_add_fetch(&__statistic.total.tx, __statistic.tx_size, __ATOMIC_SEQ_CST);
	__atomic_add_fetch(&__statistic.range.tx, __statistic.tx_size, __ATOMIC_SEQ_CST);
}

static void STDCALL cli_callback(const struct nis_event *event, const void *data)
{
	tcp_data_t *tcpdata = (tcp_data_t *)data;

	if (event->Event == EVT_CLOSED) {
		posix__release_waitable_handle(__statistic.wait);		/* notify main thread exit */
		return;
	}

	if (EVT_RECEIVEDATA == event->Event) {
		srv_receive_data(event->Ln.Tcp.Link, tcpdata->e.Packet.Data, tcpdata->e.Packet.Size);
		return;
	}
}

static char *cli_speed_format(char *holder, int64_t n)
{
	int off;

	off = 0;

	if (n < (1 << 10)) {
		off = sprintf(holder, INT64_STRFMT" Bps", n );
	} else {
		if (n < (1 << 20)) {
			off = sprintf(holder, "%.2f KBps", (double)n / (1 << 10) );
		} else {
			if (n < (1 << 30)) {
				off = sprintf(holder, "%.2f MBps", (double)n / (1 << 20) );
			} else {
				off = sprintf(holder, "%.2f GBps", (double)n / (1 << 30) );
			}
		}
	}

	holder[off] = 0;
	return holder;
}

int cli_startup()
{
	struct misc_endpoint endpoint;
	HTCPLINK link;
	tst_t tst;
	int window;
	int interval;
	int n;
	struct stat_item range;
	int64_t rtt;
	int64_t total_io_n;
	char rx_holder[32], tx_holder[32];
	int txn;
	int seq;

	arg_duplicate_client_argument(&endpoint, &txn, &interval, &window);

	link = tcp_create(&cli_callback, NULL, 0);
	if (link < 0) {
		misc_alarm_error("fails create TCP link as a client" );
		return -1;
	}

	tst.parser_ = &tst_parser;
    tst.builder_ = &tst_builder;
    tst.cb_ = sizeof(head_t);
    nis_cntl(link, NI_SETTST, &tst);

    if (tcp_connect(link, endpoint.host, endpoint.port) < 0) {
    	misc_alarm_error("fails connect to target %s:%d", endpoint.host, endpoint.port );
    	return -1;
    }

    __statistic.tx_size = sizeof(struct escape_head) + txn;
    __statistic.tx_buffer = malloc(__statistic.tx_size);
    assert(__statistic.tx_buffer);

    seq = 0;
    do {
    	((struct escape_head *)__statistic.tx_buffer)->seq = seq++;
    	((struct escape_head *)__statistic.tx_buffer)->tx_timestamp = posix__clock_monotonic();
    	n = tcp_write(link, __statistic.tx_buffer, __statistic.tx_size, NULL);
    	if (n <= 0) {
    		return -1;
    	}

    	__atomic_add_fetch(&__statistic.total.io, 1, __ATOMIC_SEQ_CST);
    	__atomic_add_fetch(&__statistic.total.tx, __statistic.tx_size, __ATOMIC_SEQ_CST);
    	__atomic_add_fetch(&__statistic.range.io, 1, __ATOMIC_SEQ_CST);
    	__atomic_add_fetch(&__statistic.range.tx, __statistic.tx_size, __ATOMIC_SEQ_CST);
    } while (--window > 0);

    printf("ios\tiops\ttx\t\trx\t\trtt\n");
    posix__allocate_synchronous_waitable_handle(&__statistic.wait);
    while (ETIMEDOUT == posix__waitfor_waitable_handle(__statistic.wait, interval)) {
    	total_io_n = __atomic_load_n(&__statistic.total.io, __ATOMIC_SEQ_CST);
    	range.io = __atomic_exchange_n(&__statistic.range.io, 0, __ATOMIC_SEQ_CST);
    	range.rx = __atomic_exchange_n(&__statistic.range.rx, 0, __ATOMIC_SEQ_CST);
    	range.tx = __atomic_exchange_n(&__statistic.range.tx, 0, __ATOMIC_SEQ_CST);
    	rtt = __atomic_exchange_n(&__statistic.rtt, 0, __ATOMIC_SEQ_CST);

    	cli_speed_format(tx_holder, range.tx);
    	cli_speed_format(rx_holder, range.rx);
    	printf(INT64_STRFMT"\t"INT64_STRFMT"\t%s\t%s\t%.2f\n", total_io_n, range.io, tx_holder, rx_holder, (double)rtt / 10000);
    }

    udp_destroy(link);
    free(__statistic.tx_buffer);
    return 0;
}
