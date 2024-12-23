#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_ecall_init_t {
	int ms_retval;
	uint16_t ms_port;
} ms_ecall_init_t;

typedef struct ms_ecall_destroy_t {
	int ms_retval;
	uint16_t ms_port;
} ms_ecall_destroy_t;

typedef struct ms_ecall_stop_t {
	int ms_retval;
	uint16_t ms_port;
} ms_ecall_stop_t;

typedef struct ms_ecall_monitor_t {
	int ms_retval;
	uint16_t ms_port;
} ms_ecall_monitor_t;

typedef struct ms_ecall_loop_recvfrom_t {
	int ms_retval;
	uint16_t ms_port;
} ms_ecall_loop_recvfrom_t;

typedef struct ms_ecall_refresh_t {
	int ms_retval;
	uint16_t ms_port;
} ms_ecall_refresh_t;

typedef struct ms_ecall_untaint_trigger_t {
	int ms_retval;
	uint16_t ms_port;
} ms_ecall_untaint_trigger_t;

typedef struct ms_ecall_add_sibling_t {
	int ms_retval;
	uint16_t ms_port;
	const char* ms_sibling_ip;
	size_t ms_sibling_ip_len;
	uint16_t ms_sibling_port;
} ms_ecall_add_sibling_t;

typedef struct ms_ecall_get_timestamp_t {
	int ms_retval;
	uint16_t ms_port;
	struct timespec* ms_timestamp;
} ms_ecall_get_timestamp_t;

typedef struct ms_ecall_init_prov_t {
	int ms_retval;
	uint16_t ms__prov_port;
	int ms__service_time;
	uint16_t ms__triad_port;
} ms_ecall_init_prov_t;

typedef struct ms_ecall_destroy_prov_t {
	int ms_retval;
	uint16_t ms__prov_port;
} ms_ecall_destroy_prov_t;

typedef struct ms_ecall_stop_prov_t {
	int ms_retval;
	uint16_t ms__prov_port;
} ms_ecall_stop_prov_t;

typedef struct ms_ecall_loop_recvfrom_prov_t {
	int ms_retval;
	uint16_t ms__prov_port;
} ms_ecall_loop_recvfrom_prov_t;

typedef struct ms_ecall_loop_handle_rq_prov_t {
	int ms_retval;
	uint16_t ms__prov_port;
} ms_ecall_loop_handle_rq_prov_t;

typedef struct ms_sl_init_switchless_t {
	sgx_status_t ms_retval;
	void* ms_sl_data;
} ms_sl_init_switchless_t;

typedef struct ms_sl_run_switchless_tworker_t {
	sgx_status_t ms_retval;
} ms_sl_run_switchless_tworker_t;

typedef struct ms_ocall_print_string_t {
	const char* ms_str;
} ms_ocall_print_string_t;

typedef struct ms_ocall_readTSC_t {
	long long* ms_timestamps;
} ms_ocall_readTSC_t;

typedef struct ms_ocall_sleep_t {
	int ms_time;
} ms_ocall_sleep_t;

typedef struct ms_ocall_usleep_t {
	int ms_time;
} ms_ocall_usleep_t;

typedef struct ms_ocall_timespec_get_t {
	struct timespec* ms_ts;
} ms_ocall_timespec_get_t;

typedef struct ms_ocall_timespec_print_t {
	struct timespec* ms_ts;
} ms_ocall_timespec_print_t;

typedef struct ms_sgx_oc_cpuidex_t {
	int* ms_cpuinfo;
	int ms_leaf;
	int ms_subleaf;
} ms_sgx_oc_cpuidex_t;

typedef struct ms_sgx_thread_wait_untrusted_event_ocall_t {
	int ms_retval;
	const void* ms_self;
} ms_sgx_thread_wait_untrusted_event_ocall_t;

typedef struct ms_sgx_thread_set_untrusted_event_ocall_t {
	int ms_retval;
	const void* ms_waiter;
} ms_sgx_thread_set_untrusted_event_ocall_t;

typedef struct ms_sgx_thread_setwait_untrusted_events_ocall_t {
	int ms_retval;
	const void* ms_waiter;
	const void* ms_self;
} ms_sgx_thread_setwait_untrusted_events_ocall_t;

typedef struct ms_sgx_thread_set_multiple_untrusted_events_ocall_t {
	int ms_retval;
	const void** ms_waiters;
	size_t ms_total;
} ms_sgx_thread_set_multiple_untrusted_events_ocall_t;

typedef struct ms_pthread_wait_timeout_ocall_t {
	int ms_retval;
	unsigned long long ms_waiter;
	unsigned long long ms_timeout;
} ms_pthread_wait_timeout_ocall_t;

typedef struct ms_pthread_create_ocall_t {
	int ms_retval;
	unsigned long long ms_self;
} ms_pthread_create_ocall_t;

typedef struct ms_pthread_wakeup_ocall_t {
	int ms_retval;
	unsigned long long ms_waiter;
} ms_pthread_wakeup_ocall_t;

typedef struct ms_u_socket_t {
	int ms_retval;
	int ocall_errno;
	int ms_domain;
	int ms_type;
	int ms_protocol;
} ms_u_socket_t;

typedef struct ms_u_bind_t {
	int ms_retval;
	int ocall_errno;
	int ms_fd;
	const struct sockaddr* ms_addr;
	socklen_t ms_len;
} ms_u_bind_t;

typedef struct ms_u_listen_t {
	int ms_retval;
	int ocall_errno;
	int ms_fd;
	int ms_n;
} ms_u_listen_t;

typedef struct ms_u_accept_t {
	int ms_retval;
	int ocall_errno;
	int ms_fd;
	struct sockaddr* ms_addr;
	socklen_t ms_addrlen_in;
	socklen_t* ms_addrlen_out;
} ms_u_accept_t;

typedef struct ms_u_connect_t {
	int ms_retval;
	int ocall_errno;
	int ms_fd;
	const struct sockaddr* ms_addr;
	socklen_t ms_len;
} ms_u_connect_t;

typedef struct ms_u_send_t {
	ssize_t ms_retval;
	int ocall_errno;
	int ms_fd;
	const void* ms_buf;
	size_t ms_n;
	int ms_flags;
} ms_u_send_t;

typedef struct ms_u_recv_t {
	ssize_t ms_retval;
	int ocall_errno;
	int ms_fd;
	void* ms_buf;
	size_t ms_n;
	int ms_flags;
} ms_u_recv_t;

typedef struct ms_u_sendto_t {
	ssize_t ms_retval;
	int ocall_errno;
	int ms_fd;
	const void* ms_buf;
	size_t ms_n;
	int ms_flags;
	const char* ms_ip;
	size_t ms_iplen;
	int ms_port;
} ms_u_sendto_t;

typedef struct ms_u_recvfrom_t {
	ssize_t ms_retval;
	int ocall_errno;
	int ms_fd;
	void* ms_buf;
	size_t ms_n;
	int ms_flags;
	char* ms_ip;
	size_t ms_iplen;
	int* ms_port;
} ms_u_recvfrom_t;

typedef struct ms_u_setsockopt_t {
	int ms_retval;
	int ocall_errno;
	int ms_fd;
	int ms_level;
	int ms_optname;
	const void* ms_optval;
	socklen_t ms_optlen;
} ms_u_setsockopt_t;

typedef struct ms_u_close_t {
	int ms_retval;
	int ocall_errno;
	int ms_fd;
} ms_u_close_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print_string(void* pms)
{
	ms_ocall_print_string_t* ms = SGX_CAST(ms_ocall_print_string_t*, pms);
	ocall_print_string(ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_readTSC(void* pms)
{
	ms_ocall_readTSC_t* ms = SGX_CAST(ms_ocall_readTSC_t*, pms);
	ocall_readTSC(ms->ms_timestamps);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_sleep(void* pms)
{
	ms_ocall_sleep_t* ms = SGX_CAST(ms_ocall_sleep_t*, pms);
	ocall_sleep(ms->ms_time);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_usleep(void* pms)
{
	ms_ocall_usleep_t* ms = SGX_CAST(ms_ocall_usleep_t*, pms);
	ocall_usleep(ms->ms_time);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_timespec_get(void* pms)
{
	ms_ocall_timespec_get_t* ms = SGX_CAST(ms_ocall_timespec_get_t*, pms);
	ocall_timespec_get(ms->ms_ts);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_timespec_print(void* pms)
{
	ms_ocall_timespec_print_t* ms = SGX_CAST(ms_ocall_timespec_print_t*, pms);
	ocall_timespec_print(ms->ms_ts);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_sgx_oc_cpuidex(void* pms)
{
	ms_sgx_oc_cpuidex_t* ms = SGX_CAST(ms_sgx_oc_cpuidex_t*, pms);
	sgx_oc_cpuidex(ms->ms_cpuinfo, ms->ms_leaf, ms->ms_subleaf);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_sgx_thread_wait_untrusted_event_ocall(void* pms)
{
	ms_sgx_thread_wait_untrusted_event_ocall_t* ms = SGX_CAST(ms_sgx_thread_wait_untrusted_event_ocall_t*, pms);
	ms->ms_retval = sgx_thread_wait_untrusted_event_ocall(ms->ms_self);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_sgx_thread_set_untrusted_event_ocall(void* pms)
{
	ms_sgx_thread_set_untrusted_event_ocall_t* ms = SGX_CAST(ms_sgx_thread_set_untrusted_event_ocall_t*, pms);
	ms->ms_retval = sgx_thread_set_untrusted_event_ocall(ms->ms_waiter);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_sgx_thread_setwait_untrusted_events_ocall(void* pms)
{
	ms_sgx_thread_setwait_untrusted_events_ocall_t* ms = SGX_CAST(ms_sgx_thread_setwait_untrusted_events_ocall_t*, pms);
	ms->ms_retval = sgx_thread_setwait_untrusted_events_ocall(ms->ms_waiter, ms->ms_self);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_sgx_thread_set_multiple_untrusted_events_ocall(void* pms)
{
	ms_sgx_thread_set_multiple_untrusted_events_ocall_t* ms = SGX_CAST(ms_sgx_thread_set_multiple_untrusted_events_ocall_t*, pms);
	ms->ms_retval = sgx_thread_set_multiple_untrusted_events_ocall(ms->ms_waiters, ms->ms_total);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_pthread_wait_timeout_ocall(void* pms)
{
	ms_pthread_wait_timeout_ocall_t* ms = SGX_CAST(ms_pthread_wait_timeout_ocall_t*, pms);
	ms->ms_retval = pthread_wait_timeout_ocall(ms->ms_waiter, ms->ms_timeout);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_pthread_create_ocall(void* pms)
{
	ms_pthread_create_ocall_t* ms = SGX_CAST(ms_pthread_create_ocall_t*, pms);
	ms->ms_retval = pthread_create_ocall(ms->ms_self);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_pthread_wakeup_ocall(void* pms)
{
	ms_pthread_wakeup_ocall_t* ms = SGX_CAST(ms_pthread_wakeup_ocall_t*, pms);
	ms->ms_retval = pthread_wakeup_ocall(ms->ms_waiter);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_socket(void* pms)
{
	ms_u_socket_t* ms = SGX_CAST(ms_u_socket_t*, pms);
	ms->ms_retval = u_socket(ms->ms_domain, ms->ms_type, ms->ms_protocol);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_bind(void* pms)
{
	ms_u_bind_t* ms = SGX_CAST(ms_u_bind_t*, pms);
	ms->ms_retval = u_bind(ms->ms_fd, ms->ms_addr, ms->ms_len);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_listen(void* pms)
{
	ms_u_listen_t* ms = SGX_CAST(ms_u_listen_t*, pms);
	ms->ms_retval = u_listen(ms->ms_fd, ms->ms_n);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_accept(void* pms)
{
	ms_u_accept_t* ms = SGX_CAST(ms_u_accept_t*, pms);
	ms->ms_retval = u_accept(ms->ms_fd, ms->ms_addr, ms->ms_addrlen_in, ms->ms_addrlen_out);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_connect(void* pms)
{
	ms_u_connect_t* ms = SGX_CAST(ms_u_connect_t*, pms);
	ms->ms_retval = u_connect(ms->ms_fd, ms->ms_addr, ms->ms_len);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_send(void* pms)
{
	ms_u_send_t* ms = SGX_CAST(ms_u_send_t*, pms);
	ms->ms_retval = u_send(ms->ms_fd, ms->ms_buf, ms->ms_n, ms->ms_flags);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_recv(void* pms)
{
	ms_u_recv_t* ms = SGX_CAST(ms_u_recv_t*, pms);
	ms->ms_retval = u_recv(ms->ms_fd, ms->ms_buf, ms->ms_n, ms->ms_flags);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_sendto(void* pms)
{
	ms_u_sendto_t* ms = SGX_CAST(ms_u_sendto_t*, pms);
	ms->ms_retval = u_sendto(ms->ms_fd, ms->ms_buf, ms->ms_n, ms->ms_flags, ms->ms_ip, ms->ms_iplen, ms->ms_port);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_recvfrom(void* pms)
{
	ms_u_recvfrom_t* ms = SGX_CAST(ms_u_recvfrom_t*, pms);
	ms->ms_retval = u_recvfrom(ms->ms_fd, ms->ms_buf, ms->ms_n, ms->ms_flags, ms->ms_ip, ms->ms_iplen, ms->ms_port);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_setsockopt(void* pms)
{
	ms_u_setsockopt_t* ms = SGX_CAST(ms_u_setsockopt_t*, pms);
	ms->ms_retval = u_setsockopt(ms->ms_fd, ms->ms_level, ms->ms_optname, ms->ms_optval, ms->ms_optlen);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_u_close(void* pms)
{
	ms_u_close_t* ms = SGX_CAST(ms_u_close_t*, pms);
	ms->ms_retval = u_close(ms->ms_fd);
	ms->ocall_errno = errno;
	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[25];
} ocall_table_Enclave = {
	25,
	{
		(void*)Enclave_ocall_print_string,
		(void*)Enclave_ocall_readTSC,
		(void*)Enclave_ocall_sleep,
		(void*)Enclave_ocall_usleep,
		(void*)Enclave_ocall_timespec_get,
		(void*)Enclave_ocall_timespec_print,
		(void*)Enclave_sgx_oc_cpuidex,
		(void*)Enclave_sgx_thread_wait_untrusted_event_ocall,
		(void*)Enclave_sgx_thread_set_untrusted_event_ocall,
		(void*)Enclave_sgx_thread_setwait_untrusted_events_ocall,
		(void*)Enclave_sgx_thread_set_multiple_untrusted_events_ocall,
		(void*)Enclave_pthread_wait_timeout_ocall,
		(void*)Enclave_pthread_create_ocall,
		(void*)Enclave_pthread_wakeup_ocall,
		(void*)Enclave_u_socket,
		(void*)Enclave_u_bind,
		(void*)Enclave_u_listen,
		(void*)Enclave_u_accept,
		(void*)Enclave_u_connect,
		(void*)Enclave_u_send,
		(void*)Enclave_u_recv,
		(void*)Enclave_u_sendto,
		(void*)Enclave_u_recvfrom,
		(void*)Enclave_u_setsockopt,
		(void*)Enclave_u_close,
	}
};
sgx_status_t ecall_init(sgx_enclave_id_t eid, int* retval, uint16_t port)
{
	sgx_status_t status;
	ms_ecall_init_t ms;
	ms.ms_port = port;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_destroy(sgx_enclave_id_t eid, int* retval, uint16_t port)
{
	sgx_status_t status;
	ms_ecall_destroy_t ms;
	ms.ms_port = port;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_stop(sgx_enclave_id_t eid, int* retval, uint16_t port)
{
	sgx_status_t status;
	ms_ecall_stop_t ms;
	ms.ms_port = port;
	status = sgx_ecall(eid, 2, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_monitor(sgx_enclave_id_t eid, int* retval, uint16_t port)
{
	sgx_status_t status;
	ms_ecall_monitor_t ms;
	ms.ms_port = port;
	status = sgx_ecall(eid, 3, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_loop_recvfrom(sgx_enclave_id_t eid, int* retval, uint16_t port)
{
	sgx_status_t status;
	ms_ecall_loop_recvfrom_t ms;
	ms.ms_port = port;
	status = sgx_ecall_switchless(eid, 4, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_refresh(sgx_enclave_id_t eid, int* retval, uint16_t port)
{
	sgx_status_t status;
	ms_ecall_refresh_t ms;
	ms.ms_port = port;
	status = sgx_ecall_switchless(eid, 5, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_untaint_trigger(sgx_enclave_id_t eid, int* retval, uint16_t port)
{
	sgx_status_t status;
	ms_ecall_untaint_trigger_t ms;
	ms.ms_port = port;
	status = sgx_ecall_switchless(eid, 6, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_add_sibling(sgx_enclave_id_t eid, int* retval, uint16_t port, const char* sibling_ip, uint16_t sibling_port)
{
	sgx_status_t status;
	ms_ecall_add_sibling_t ms;
	ms.ms_port = port;
	ms.ms_sibling_ip = sibling_ip;
	ms.ms_sibling_ip_len = sibling_ip ? strlen(sibling_ip) + 1 : 0;
	ms.ms_sibling_port = sibling_port;
	status = sgx_ecall_switchless(eid, 7, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_get_timestamp(sgx_enclave_id_t eid, int* retval, uint16_t port, struct timespec* timestamp)
{
	sgx_status_t status;
	ms_ecall_get_timestamp_t ms;
	ms.ms_port = port;
	ms.ms_timestamp = timestamp;
	status = sgx_ecall_switchless(eid, 8, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_init_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port, int _service_time, uint16_t _triad_port)
{
	sgx_status_t status;
	ms_ecall_init_prov_t ms;
	ms.ms__prov_port = _prov_port;
	ms.ms__service_time = _service_time;
	ms.ms__triad_port = _triad_port;
	status = sgx_ecall(eid, 9, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_destroy_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port)
{
	sgx_status_t status;
	ms_ecall_destroy_prov_t ms;
	ms.ms__prov_port = _prov_port;
	status = sgx_ecall(eid, 10, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_stop_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port)
{
	sgx_status_t status;
	ms_ecall_stop_prov_t ms;
	ms.ms__prov_port = _prov_port;
	status = sgx_ecall(eid, 11, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_loop_recvfrom_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port)
{
	sgx_status_t status;
	ms_ecall_loop_recvfrom_prov_t ms;
	ms.ms__prov_port = _prov_port;
	status = sgx_ecall_switchless(eid, 12, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t ecall_loop_handle_rq_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port)
{
	sgx_status_t status;
	ms_ecall_loop_handle_rq_prov_t ms;
	ms.ms__prov_port = _prov_port;
	status = sgx_ecall_switchless(eid, 13, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t sl_init_switchless(sgx_enclave_id_t eid, sgx_status_t* retval, void* sl_data)
{
	sgx_status_t status;
	ms_sl_init_switchless_t ms;
	ms.ms_sl_data = sl_data;
	status = sgx_ecall(eid, 14, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t sl_run_switchless_tworker(sgx_enclave_id_t eid, sgx_status_t* retval)
{
	sgx_status_t status;
	ms_sl_run_switchless_tworker_t ms;
	status = sgx_ecall(eid, 15, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

