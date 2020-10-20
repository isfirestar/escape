#include "nis.h"
#include "args.h"
#include "proto.h"

#include "posix_wait.h"
#include "posix_time.h"
#include "posix_thread.h"
#include "posix_naos.h"

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
	posix__waitable_handle_t *wait;
};
static struct statistic __statistic = { .total = { 0, 0, 0 }, .range = { 0, 0, 0 }, .rtt = 0, .wait = NULL };

struct sender_context
{
	posix__pthread_t thread;
	posix__waitable_handle_t *wait;
	int refs;
	HTCPLINK link;
	unsigned char *tx_buffer;
	int seq;
	int tx_size;
};
static struct sender_context __sender_context = { .wait = NULL, .refs = 0, .link = INVALID_HTCPLINK, .tx_buffer = NULL, .seq = 0, .tx_size = 0};

#define _USE_SENDER_THREAD (0)

static void cli_receive_data(HTCPLINK link, const unsigned char *data, int len)
{
	int64_t timestamp;
	int64_t rtt, rtt_max;

	timestamp = posix__clock_monotonic();

	__atomic_add_fetch(&__statistic.total.io, 1, __ATOMIC_SEQ_CST);
	__atomic_add_fetch(&__statistic.range.io, 1, __ATOMIC_SEQ_CST);

	__atomic_add_fetch(&__statistic.range.rx, len + sizeof(struct nsp_head), __ATOMIC_SEQ_CST);

	rtt = timestamp - ((const struct escape_head *)data)->tx_timestamp;
	rtt_max = __atomic_load_n(&__statistic.rtt, __ATOMIC_SEQ_CST);
	if (0 == rtt_max || rtt > rtt_max) {
		__atomic_store_n(&__statistic.rtt, rtt, __ATOMIC_SEQ_CST);
	}

#if _USE_SENDER_THREAD
	__atomic_add_fetch(&__sender_context.refs, 1, __ATOMIC_SEQ_CST);
	posix__sig_waitable_handle(__sender_context.wait);
#else
	((struct escape_head *)__sender_context.tx_buffer)->seq = ((const struct escape_head *)data)->seq + 1;
    ((struct escape_head *)__sender_context.tx_buffer)->tx_timestamp = posix__clock_monotonic();

	if ( tcp_write(link, __sender_context.tx_buffer, __sender_context.tx_size, NULL) < 0 ) {
		misc_alarm_error("[cli_receive_data] fails on tcp_write." );
		posix__release_waitable_handle(__statistic.wait);
		return;
	}

	__atomic_add_fetch(&__statistic.total.tx, __sender_context.tx_size + sizeof(struct nsp_head), __ATOMIC_SEQ_CST);
	__atomic_add_fetch(&__statistic.range.tx, __sender_context.tx_size + sizeof(struct nsp_head), __ATOMIC_SEQ_CST);
#endif
}

static void STDCALL cli_callback(const struct nis_event *event, const void *data)
{
	tcp_data_t *tcpdata = (tcp_data_t *)data;

	if (event->Event == EVT_CLOSED) {
		misc_alarm_error("[cli_callback] link closed." );
		posix__release_waitable_handle(__statistic.wait);		/* notify main thread exit */
		return;
	}

	if (EVT_RECEIVEDATA == event->Event) {
		cli_receive_data(event->Ln.Tcp.Link, tcpdata->e.Packet.Data, tcpdata->e.Packet.Size);
		return;
	}
}

static char *cli_speed_format(char *holder, int64_t n)
{
	int off;

	off = 0;
	holder[0] = 0;

	if (n < (1 << 10)) {
		off = sprintf(holder, INT64_STRFMT" Bps", n );
	} else {
		if (n < (1 << 20)) {
			off = sprintf(holder, "%.2f KBps", (double)n / (1 << 10) );
		} else {
			off = sprintf(holder, "%.2f MBps", (double)n / (1 << 20) );
		}
	}

	holder[off] = 0;
	return holder;
}

void *sender_thread_proc(void *p)
{
	int *seq;
	int64_t *timestamp;

	seq = &((struct escape_head *)__sender_context.tx_buffer)->seq;
	timestamp = &((struct escape_head *)__sender_context.tx_buffer)->tx_timestamp;

	while (0 == posix__waitfor_waitable_handle(__sender_context.wait, -1)) {
		posix__block_waitable_handle(__sender_context.wait);
		while (__atomic_load_n(&__sender_context.refs, __ATOMIC_SEQ_CST) > 0) {
			*seq = __sender_context.seq++;
		    *timestamp = posix__clock_monotonic();

			tcp_write(__sender_context.link, __sender_context.tx_buffer, __sender_context.tx_size, NULL);

			__atomic_add_fetch(&__statistic.total.tx, __sender_context.tx_size + sizeof(struct nsp_head), __ATOMIC_SEQ_CST);
			__atomic_add_fetch(&__statistic.range.tx, __sender_context.tx_size + sizeof(struct nsp_head), __ATOMIC_SEQ_CST);

			__atomic_sub_fetch(&__sender_context.refs, 1, __ATOMIC_SEQ_CST);
		}
	}
	return NULL;
}

int cli_startup()
{
	struct misc_endpoint endpoint;
	HTCPLINK link;
	tst_t tst;
	int window;
	int n;
	struct stat_item range;
	int64_t rtt;
	int64_t total_io_n;
	char rx_holder[32], tx_holder[32];
	int txn;
	int seq;
	int timeouts;
	unsigned int local_inet;
	unsigned short local_port;
	char local_ipstr[16];

	arg_duplicate_client_argument(&endpoint, &txn, &window);
	if (window > 10) {
		misc_alarm_warning("request window count:%d is illegal and mandatory replace to default", window );
		window = 1;
	}

	link = tcp_create(&cli_callback, NULL, 0);
	if (link < 0) {
		misc_alarm_error("fails create TCP link as a client" );
		return -1;
	}

	tst.parser_ = &tst_parser;
    tst.builder_ = &tst_builder;
    tst.cb_ = sizeof(struct nsp_head);
    nis_cntl(link, NI_SETTST, &tst);

    if (tcp_connect(link, endpoint.host, endpoint.port) < 0) {
    	misc_alarm_error("fails connect to target %s:%d", endpoint.host, endpoint.port );
    	return -1;
    }

    tcp_getaddr(link, LINK_ADDR_LOCAL, &local_inet, &local_port);
    posix__ipv4tos(local_inet, local_ipstr, sizeof(local_ipstr));
    misc_alarm_trace("connected success, local session bind on %s:%d.", local_ipstr, local_port);

    __sender_context.tx_size = sizeof(struct escape_head) + txn;
    __sender_context.tx_buffer = malloc(__sender_context.tx_size);
    assert(__sender_context.tx_buffer);

#if _USE_SENDER_THREAD
    __sender_context.link = link;
    __sender_context.seq = 0;
    posix__allocate_notification_waitable_handle(&__sender_context.wait);
    posix__pthread_create(&__sender_context.thread, &sender_thread_proc, NULL);
    __atomic_add_fetch(&__sender_context.refs, window, __ATOMIC_SEQ_CST);
    posix__sig_waitable_handle(__sender_context.wait);
#else
    seq = 0;
    do {
    	((struct escape_head *)__sender_context.tx_buffer)->seq = seq++;
    	((struct escape_head *)__sender_context.tx_buffer)->tx_timestamp = posix__clock_monotonic();
    	n = tcp_write(link, __sender_context.tx_buffer, __sender_context.tx_size, NULL);
    	if (n <= 0) {
    		return -1;
    	}

    	__atomic_add_fetch(&__statistic.total.io, 1, __ATOMIC_SEQ_CST);
    	__atomic_add_fetch(&__statistic.total.tx, __sender_context.tx_size, __ATOMIC_SEQ_CST);
    	__atomic_add_fetch(&__statistic.range.io, 1, __ATOMIC_SEQ_CST);
    	__atomic_add_fetch(&__statistic.range.tx, __sender_context.tx_size, __ATOMIC_SEQ_CST);
    } while (--window > 0);
#endif

    timeouts = 0;
    printf("IOs\tIOps\tTx\t\tRx\t\trtt\n");
    posix__allocate_synchronous_waitable_handle(&__statistic.wait);
    while (ETIMEDOUT == posix__waitfor_waitable_handle(__statistic.wait, 1000)) {
    	total_io_n = __atomic_load_n(&__statistic.total.io, __ATOMIC_SEQ_CST);
    	range.io = __atomic_exchange_n(&__statistic.range.io, 0, __ATOMIC_SEQ_CST);
    	range.rx = __atomic_exchange_n(&__statistic.range.rx, 0, __ATOMIC_SEQ_CST);
    	range.tx = __atomic_exchange_n(&__statistic.range.tx, 0, __ATOMIC_SEQ_CST);
    	rtt = __atomic_exchange_n(&__statistic.rtt, 0, __ATOMIC_SEQ_CST);

    	cli_speed_format(tx_holder, range.tx);
    	cli_speed_format(rx_holder, range.rx);
    	printf(INT64_STRFMT"\t"INT64_STRFMT"\t%s\t%s\t%.2f ms\n", total_io_n, range.io, tx_holder, rx_holder, (double)rtt / 10000);

    	/* no any data transmit during 3 seconds, the link is going to initiative destroy cause by timeout */
#if 0
    	if (0 == range.rx && 0 == range.tx) {
    		if (timeouts++ >= 3) {
    			break;
    		}
    	} else {
    		timeouts = 0;
    	}
#endif

    	if ( 0 == range.io && 0 == range.rx && 0 == range.tx ) {
	    	if (tcp_write(link, __sender_context.tx_buffer, __sender_context.tx_size, NULL) <= 0) {
	    		misc_alarm_error("[client cycle] fails on tcp_write." );
	    		break;
	    	}
    	}

    }

    tcp_destroy(link);
    free(__sender_context.tx_buffer);
    return 0;
}
