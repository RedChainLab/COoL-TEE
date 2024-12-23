#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)


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

static sgx_status_t SGX_CDECL sgx_ecall_init(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_init_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_init_t* ms = SGX_CAST(ms_ecall_init_t*, pms);
	ms_ecall_init_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_init_t), ms, sizeof(ms_ecall_init_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_init(__in_ms.ms_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_destroy(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_destroy_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_destroy_t* ms = SGX_CAST(ms_ecall_destroy_t*, pms);
	ms_ecall_destroy_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_destroy_t), ms, sizeof(ms_ecall_destroy_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_destroy(__in_ms.ms_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_stop(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_stop_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_stop_t* ms = SGX_CAST(ms_ecall_stop_t*, pms);
	ms_ecall_stop_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_stop_t), ms, sizeof(ms_ecall_stop_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_stop(__in_ms.ms_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_monitor(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_monitor_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_monitor_t* ms = SGX_CAST(ms_ecall_monitor_t*, pms);
	ms_ecall_monitor_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_monitor_t), ms, sizeof(ms_ecall_monitor_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_monitor(__in_ms.ms_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_loop_recvfrom(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_loop_recvfrom_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_loop_recvfrom_t* ms = SGX_CAST(ms_ecall_loop_recvfrom_t*, pms);
	ms_ecall_loop_recvfrom_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_loop_recvfrom_t), ms, sizeof(ms_ecall_loop_recvfrom_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_loop_recvfrom(__in_ms.ms_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_refresh(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_refresh_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_refresh_t* ms = SGX_CAST(ms_ecall_refresh_t*, pms);
	ms_ecall_refresh_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_refresh_t), ms, sizeof(ms_ecall_refresh_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_refresh(__in_ms.ms_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_untaint_trigger(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_untaint_trigger_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_untaint_trigger_t* ms = SGX_CAST(ms_ecall_untaint_trigger_t*, pms);
	ms_ecall_untaint_trigger_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_untaint_trigger_t), ms, sizeof(ms_ecall_untaint_trigger_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_untaint_trigger(__in_ms.ms_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_add_sibling(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_add_sibling_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_add_sibling_t* ms = SGX_CAST(ms_ecall_add_sibling_t*, pms);
	ms_ecall_add_sibling_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_add_sibling_t), ms, sizeof(ms_ecall_add_sibling_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	const char* _tmp_sibling_ip = __in_ms.ms_sibling_ip;
	size_t _len_sibling_ip = __in_ms.ms_sibling_ip_len ;
	char* _in_sibling_ip = NULL;
	int _in_retval;

	CHECK_UNIQUE_POINTER(_tmp_sibling_ip, _len_sibling_ip);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_sibling_ip != NULL && _len_sibling_ip != 0) {
		_in_sibling_ip = (char*)malloc(_len_sibling_ip);
		if (_in_sibling_ip == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_sibling_ip, _len_sibling_ip, _tmp_sibling_ip, _len_sibling_ip)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

		_in_sibling_ip[_len_sibling_ip - 1] = '\0';
		if (_len_sibling_ip != strlen(_in_sibling_ip) + 1)
		{
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	_in_retval = ecall_add_sibling(__in_ms.ms_port, (const char*)_in_sibling_ip, __in_ms.ms_sibling_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	if (_in_sibling_ip) free(_in_sibling_ip);
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_get_timestamp(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_get_timestamp_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_get_timestamp_t* ms = SGX_CAST(ms_ecall_get_timestamp_t*, pms);
	ms_ecall_get_timestamp_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_get_timestamp_t), ms, sizeof(ms_ecall_get_timestamp_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	struct timespec* _tmp_timestamp = __in_ms.ms_timestamp;
	size_t _len_timestamp = sizeof(struct timespec);
	struct timespec* _in_timestamp = NULL;
	int _in_retval;

	CHECK_UNIQUE_POINTER(_tmp_timestamp, _len_timestamp);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_timestamp != NULL && _len_timestamp != 0) {
		if ((_in_timestamp = (struct timespec*)malloc(_len_timestamp)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_timestamp, 0, _len_timestamp);
	}
	_in_retval = ecall_get_timestamp(__in_ms.ms_port, _in_timestamp);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}
	if (_in_timestamp) {
		if (memcpy_verw_s(_tmp_timestamp, _len_timestamp, _in_timestamp, _len_timestamp)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_timestamp) free(_in_timestamp);
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_init_prov(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_init_prov_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_init_prov_t* ms = SGX_CAST(ms_ecall_init_prov_t*, pms);
	ms_ecall_init_prov_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_init_prov_t), ms, sizeof(ms_ecall_init_prov_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_init_prov(__in_ms.ms__prov_port, __in_ms.ms__service_time, __in_ms.ms__triad_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_destroy_prov(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_destroy_prov_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_destroy_prov_t* ms = SGX_CAST(ms_ecall_destroy_prov_t*, pms);
	ms_ecall_destroy_prov_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_destroy_prov_t), ms, sizeof(ms_ecall_destroy_prov_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_destroy_prov(__in_ms.ms__prov_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_stop_prov(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_stop_prov_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_stop_prov_t* ms = SGX_CAST(ms_ecall_stop_prov_t*, pms);
	ms_ecall_stop_prov_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_stop_prov_t), ms, sizeof(ms_ecall_stop_prov_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_stop_prov(__in_ms.ms__prov_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_loop_recvfrom_prov(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_loop_recvfrom_prov_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_loop_recvfrom_prov_t* ms = SGX_CAST(ms_ecall_loop_recvfrom_prov_t*, pms);
	ms_ecall_loop_recvfrom_prov_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_loop_recvfrom_prov_t), ms, sizeof(ms_ecall_loop_recvfrom_prov_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_loop_recvfrom_prov(__in_ms.ms__prov_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_loop_handle_rq_prov(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_loop_handle_rq_prov_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_loop_handle_rq_prov_t* ms = SGX_CAST(ms_ecall_loop_handle_rq_prov_t*, pms);
	ms_ecall_loop_handle_rq_prov_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_loop_handle_rq_prov_t), ms, sizeof(ms_ecall_loop_handle_rq_prov_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	int _in_retval;


	_in_retval = ecall_loop_handle_rq_prov(__in_ms.ms__prov_port);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_sl_init_switchless(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_sl_init_switchless_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_sl_init_switchless_t* ms = SGX_CAST(ms_sl_init_switchless_t*, pms);
	ms_sl_init_switchless_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_sl_init_switchless_t), ms, sizeof(ms_sl_init_switchless_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	void* _tmp_sl_data = __in_ms.ms_sl_data;
	sgx_status_t _in_retval;


	_in_retval = sl_init_switchless(_tmp_sl_data);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

static sgx_status_t SGX_CDECL sgx_sl_run_switchless_tworker(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_sl_run_switchless_tworker_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_sl_run_switchless_tworker_t* ms = SGX_CAST(ms_sl_run_switchless_tworker_t*, pms);
	ms_sl_run_switchless_tworker_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_sl_run_switchless_tworker_t), ms, sizeof(ms_sl_run_switchless_tworker_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	sgx_status_t _in_retval;


	_in_retval = sl_run_switchless_tworker();
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}

err:
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv; uint8_t is_switchless;} ecall_table[16];
} g_ecall_table = {
	16,
	{
		{(void*)(uintptr_t)sgx_ecall_init, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_destroy, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_stop, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_monitor, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_loop_recvfrom, 0, 1},
		{(void*)(uintptr_t)sgx_ecall_refresh, 0, 1},
		{(void*)(uintptr_t)sgx_ecall_untaint_trigger, 0, 1},
		{(void*)(uintptr_t)sgx_ecall_add_sibling, 0, 1},
		{(void*)(uintptr_t)sgx_ecall_get_timestamp, 0, 1},
		{(void*)(uintptr_t)sgx_ecall_init_prov, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_destroy_prov, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_stop_prov, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_loop_recvfrom_prov, 0, 1},
		{(void*)(uintptr_t)sgx_ecall_loop_handle_rq_prov, 0, 1},
		{(void*)(uintptr_t)sgx_sl_init_switchless, 0, 0},
		{(void*)(uintptr_t)sgx_sl_run_switchless_tworker, 0, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[25][16];
} g_dyn_entry_table = {
	25,
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_print_string(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_string_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_string_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_string_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_string_t));
	ocalloc_size -= sizeof(ms_ocall_print_string_t);

	if (str != NULL) {
		if (memcpy_verw_s(&ms->ms_str, sizeof(const char*), &__tmp, sizeof(const char*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}

	status = sgx_ocall_switchless(0, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_readTSC(long long* timestamps)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_timestamps = sizeof(long long);

	ms_ocall_readTSC_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_readTSC_t);
	void *__tmp = NULL;

	void *__tmp_timestamps = NULL;

	CHECK_ENCLAVE_POINTER(timestamps, _len_timestamps);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (timestamps != NULL) ? _len_timestamps : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_readTSC_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_readTSC_t));
	ocalloc_size -= sizeof(ms_ocall_readTSC_t);

	if (timestamps != NULL) {
		if (memcpy_verw_s(&ms->ms_timestamps, sizeof(long long*), &__tmp, sizeof(long long*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_timestamps = __tmp;
		if (_len_timestamps % sizeof(*timestamps) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, timestamps, _len_timestamps)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_timestamps);
		ocalloc_size -= _len_timestamps;
	} else {
		ms->ms_timestamps = NULL;
	}

	status = sgx_ocall_switchless(1, ms);

	if (status == SGX_SUCCESS) {
		if (timestamps) {
			if (memcpy_s((void*)timestamps, _len_timestamps, __tmp_timestamps, _len_timestamps)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_sleep(int time)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_sleep_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_sleep_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_sleep_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_sleep_t));
	ocalloc_size -= sizeof(ms_ocall_sleep_t);

	if (memcpy_verw_s(&ms->ms_time, sizeof(ms->ms_time), &time, sizeof(time))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall_switchless(2, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_usleep(int time)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_usleep_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_usleep_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_usleep_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_usleep_t));
	ocalloc_size -= sizeof(ms_ocall_usleep_t);

	if (memcpy_verw_s(&ms->ms_time, sizeof(ms->ms_time), &time, sizeof(time))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall_switchless(3, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_timespec_get(struct timespec* ts)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ts = sizeof(struct timespec);

	ms_ocall_timespec_get_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_timespec_get_t);
	void *__tmp = NULL;

	void *__tmp_ts = NULL;

	CHECK_ENCLAVE_POINTER(ts, _len_ts);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ts != NULL) ? _len_ts : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_timespec_get_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_timespec_get_t));
	ocalloc_size -= sizeof(ms_ocall_timespec_get_t);

	if (ts != NULL) {
		if (memcpy_verw_s(&ms->ms_ts, sizeof(struct timespec*), &__tmp, sizeof(struct timespec*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_ts = __tmp;
		memset_verw(__tmp_ts, 0, _len_ts);
		__tmp = (void *)((size_t)__tmp + _len_ts);
		ocalloc_size -= _len_ts;
	} else {
		ms->ms_ts = NULL;
	}

	status = sgx_ocall_switchless(4, ms);

	if (status == SGX_SUCCESS) {
		if (ts) {
			if (memcpy_s((void*)ts, _len_ts, __tmp_ts, _len_ts)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_timespec_print(struct timespec* ts)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_ts = sizeof(struct timespec);

	ms_ocall_timespec_print_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_timespec_print_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(ts, _len_ts);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ts != NULL) ? _len_ts : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_timespec_print_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_timespec_print_t));
	ocalloc_size -= sizeof(ms_ocall_timespec_print_t);

	if (ts != NULL) {
		if (memcpy_verw_s(&ms->ms_ts, sizeof(struct timespec*), &__tmp, sizeof(struct timespec*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, ts, _len_ts)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_ts);
		ocalloc_size -= _len_ts;
	} else {
		ms->ms_ts = NULL;
	}

	status = sgx_ocall_switchless(5, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_cpuinfo = 4 * sizeof(int);

	ms_sgx_oc_cpuidex_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_oc_cpuidex_t);
	void *__tmp = NULL;

	void *__tmp_cpuinfo = NULL;

	CHECK_ENCLAVE_POINTER(cpuinfo, _len_cpuinfo);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (cpuinfo != NULL) ? _len_cpuinfo : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_oc_cpuidex_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_oc_cpuidex_t));
	ocalloc_size -= sizeof(ms_sgx_oc_cpuidex_t);

	if (cpuinfo != NULL) {
		if (memcpy_verw_s(&ms->ms_cpuinfo, sizeof(int*), &__tmp, sizeof(int*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_cpuinfo = __tmp;
		if (_len_cpuinfo % sizeof(*cpuinfo) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset_verw(__tmp_cpuinfo, 0, _len_cpuinfo);
		__tmp = (void *)((size_t)__tmp + _len_cpuinfo);
		ocalloc_size -= _len_cpuinfo;
	} else {
		ms->ms_cpuinfo = NULL;
	}

	if (memcpy_verw_s(&ms->ms_leaf, sizeof(ms->ms_leaf), &leaf, sizeof(leaf))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_subleaf, sizeof(ms->ms_subleaf), &subleaf, sizeof(subleaf))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(6, ms);

	if (status == SGX_SUCCESS) {
		if (cpuinfo) {
			if (memcpy_s((void*)cpuinfo, _len_cpuinfo, __tmp_cpuinfo, _len_cpuinfo)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_wait_untrusted_event_ocall(int* retval, const void* self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_wait_untrusted_event_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_wait_untrusted_event_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t));
	ocalloc_size -= sizeof(ms_sgx_thread_wait_untrusted_event_ocall_t);

	if (memcpy_verw_s(&ms->ms_self, sizeof(ms->ms_self), &self, sizeof(self))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(7, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_set_untrusted_event_ocall(int* retval, const void* waiter)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_set_untrusted_event_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_set_untrusted_event_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_set_untrusted_event_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_set_untrusted_event_ocall_t));
	ocalloc_size -= sizeof(ms_sgx_thread_set_untrusted_event_ocall_t);

	if (memcpy_verw_s(&ms->ms_waiter, sizeof(ms->ms_waiter), &waiter, sizeof(waiter))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(8, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_setwait_untrusted_events_ocall(int* retval, const void* waiter, const void* self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_sgx_thread_setwait_untrusted_events_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_setwait_untrusted_events_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t));
	ocalloc_size -= sizeof(ms_sgx_thread_setwait_untrusted_events_ocall_t);

	if (memcpy_verw_s(&ms->ms_waiter, sizeof(ms->ms_waiter), &waiter, sizeof(waiter))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_self, sizeof(ms->ms_self), &self, sizeof(self))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(9, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL sgx_thread_set_multiple_untrusted_events_ocall(int* retval, const void** waiters, size_t total)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_waiters = total * sizeof(void*);

	ms_sgx_thread_set_multiple_untrusted_events_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(waiters, _len_waiters);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (waiters != NULL) ? _len_waiters : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_sgx_thread_set_multiple_untrusted_events_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t));
	ocalloc_size -= sizeof(ms_sgx_thread_set_multiple_untrusted_events_ocall_t);

	if (waiters != NULL) {
		if (memcpy_verw_s(&ms->ms_waiters, sizeof(const void**), &__tmp, sizeof(const void**))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_waiters % sizeof(*waiters) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, waiters, _len_waiters)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_waiters);
		ocalloc_size -= _len_waiters;
	} else {
		ms->ms_waiters = NULL;
	}

	if (memcpy_verw_s(&ms->ms_total, sizeof(ms->ms_total), &total, sizeof(total))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(10, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL pthread_wait_timeout_ocall(int* retval, unsigned long long waiter, unsigned long long timeout)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_pthread_wait_timeout_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_pthread_wait_timeout_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_pthread_wait_timeout_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_pthread_wait_timeout_ocall_t));
	ocalloc_size -= sizeof(ms_pthread_wait_timeout_ocall_t);

	if (memcpy_verw_s(&ms->ms_waiter, sizeof(ms->ms_waiter), &waiter, sizeof(waiter))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_timeout, sizeof(ms->ms_timeout), &timeout, sizeof(timeout))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(11, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL pthread_create_ocall(int* retval, unsigned long long self)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_pthread_create_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_pthread_create_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_pthread_create_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_pthread_create_ocall_t));
	ocalloc_size -= sizeof(ms_pthread_create_ocall_t);

	if (memcpy_verw_s(&ms->ms_self, sizeof(ms->ms_self), &self, sizeof(self))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(12, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL pthread_wakeup_ocall(int* retval, unsigned long long waiter)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_pthread_wakeup_ocall_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_pthread_wakeup_ocall_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_pthread_wakeup_ocall_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_pthread_wakeup_ocall_t));
	ocalloc_size -= sizeof(ms_pthread_wakeup_ocall_t);

	if (memcpy_verw_s(&ms->ms_waiter, sizeof(ms->ms_waiter), &waiter, sizeof(waiter))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(13, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_socket(int* retval, int domain, int type, int protocol)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_u_socket_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_socket_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_socket_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_socket_t));
	ocalloc_size -= sizeof(ms_u_socket_t);

	if (memcpy_verw_s(&ms->ms_domain, sizeof(ms->ms_domain), &domain, sizeof(domain))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_type, sizeof(ms->ms_type), &type, sizeof(type))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_protocol, sizeof(ms->ms_protocol), &protocol, sizeof(protocol))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall_switchless(14, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_bind(int* retval, int fd, const struct sockaddr* addr, socklen_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_addr = len;

	ms_u_bind_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_bind_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(addr, _len_addr);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addr != NULL) ? _len_addr : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_bind_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_bind_t));
	ocalloc_size -= sizeof(ms_u_bind_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (addr != NULL) {
		if (memcpy_verw_s(&ms->ms_addr, sizeof(const struct sockaddr*), &__tmp, sizeof(const struct sockaddr*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, addr, _len_addr)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_addr);
		ocalloc_size -= _len_addr;
	} else {
		ms->ms_addr = NULL;
	}

	if (memcpy_verw_s(&ms->ms_len, sizeof(ms->ms_len), &len, sizeof(len))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall_switchless(15, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_listen(int* retval, int fd, int n)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_u_listen_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_listen_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_listen_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_listen_t));
	ocalloc_size -= sizeof(ms_u_listen_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_n, sizeof(ms->ms_n), &n, sizeof(n))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(16, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_accept(int* retval, int fd, struct sockaddr* addr, socklen_t addrlen_in, socklen_t* addrlen_out)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_addr = addrlen_in;
	size_t _len_addrlen_out = sizeof(socklen_t);

	ms_u_accept_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_accept_t);
	void *__tmp = NULL;

	void *__tmp_addr = NULL;
	void *__tmp_addrlen_out = NULL;

	CHECK_ENCLAVE_POINTER(addr, _len_addr);
	CHECK_ENCLAVE_POINTER(addrlen_out, _len_addrlen_out);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addr != NULL) ? _len_addr : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addrlen_out != NULL) ? _len_addrlen_out : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_accept_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_accept_t));
	ocalloc_size -= sizeof(ms_u_accept_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (addr != NULL) {
		if (memcpy_verw_s(&ms->ms_addr, sizeof(struct sockaddr*), &__tmp, sizeof(struct sockaddr*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_addr = __tmp;
		memset_verw(__tmp_addr, 0, _len_addr);
		__tmp = (void *)((size_t)__tmp + _len_addr);
		ocalloc_size -= _len_addr;
	} else {
		ms->ms_addr = NULL;
	}

	if (memcpy_verw_s(&ms->ms_addrlen_in, sizeof(ms->ms_addrlen_in), &addrlen_in, sizeof(addrlen_in))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (addrlen_out != NULL) {
		if (memcpy_verw_s(&ms->ms_addrlen_out, sizeof(socklen_t*), &__tmp, sizeof(socklen_t*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_addrlen_out = __tmp;
		memset_verw(__tmp_addrlen_out, 0, _len_addrlen_out);
		__tmp = (void *)((size_t)__tmp + _len_addrlen_out);
		ocalloc_size -= _len_addrlen_out;
	} else {
		ms->ms_addrlen_out = NULL;
	}

	status = sgx_ocall(17, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (addr) {
			if (memcpy_s((void*)addr, _len_addr, __tmp_addr, _len_addr)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (addrlen_out) {
			if (memcpy_s((void*)addrlen_out, _len_addrlen_out, __tmp_addrlen_out, _len_addrlen_out)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_connect(int* retval, int fd, const struct sockaddr* addr, socklen_t len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_addr = len;

	ms_u_connect_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_connect_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(addr, _len_addr);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (addr != NULL) ? _len_addr : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_connect_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_connect_t));
	ocalloc_size -= sizeof(ms_u_connect_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (addr != NULL) {
		if (memcpy_verw_s(&ms->ms_addr, sizeof(const struct sockaddr*), &__tmp, sizeof(const struct sockaddr*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, addr, _len_addr)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_addr);
		ocalloc_size -= _len_addr;
	} else {
		ms->ms_addr = NULL;
	}

	if (memcpy_verw_s(&ms->ms_len, sizeof(ms->ms_len), &len, sizeof(len))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(18, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_send(ssize_t* retval, int fd, const void* buf, size_t n, int flags)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = n;

	ms_u_send_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_send_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_send_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_send_t));
	ocalloc_size -= sizeof(ms_u_send_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (buf != NULL) {
		if (memcpy_verw_s(&ms->ms_buf, sizeof(const void*), &__tmp, sizeof(const void*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, buf, _len_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}

	if (memcpy_verw_s(&ms->ms_n, sizeof(ms->ms_n), &n, sizeof(n))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_flags, sizeof(ms->ms_flags), &flags, sizeof(flags))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(19, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_recv(ssize_t* retval, int fd, void* buf, size_t n, int flags)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = n;

	ms_u_recv_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_recv_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_recv_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_recv_t));
	ocalloc_size -= sizeof(ms_u_recv_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (buf != NULL) {
		if (memcpy_verw_s(&ms->ms_buf, sizeof(void*), &__tmp, sizeof(void*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_buf = __tmp;
		memset_verw(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}

	if (memcpy_verw_s(&ms->ms_n, sizeof(ms->ms_n), &n, sizeof(n))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_flags, sizeof(ms->ms_flags), &flags, sizeof(flags))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(20, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_sendto(ssize_t* retval, int fd, const void* buf, size_t n, int flags, const char* ip, size_t iplen, int port)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = n;
	size_t _len_ip = iplen;

	ms_u_sendto_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_sendto_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(buf, _len_buf);
	CHECK_ENCLAVE_POINTER(ip, _len_ip);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ip != NULL) ? _len_ip : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_sendto_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_sendto_t));
	ocalloc_size -= sizeof(ms_u_sendto_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (buf != NULL) {
		if (memcpy_verw_s(&ms->ms_buf, sizeof(const void*), &__tmp, sizeof(const void*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, buf, _len_buf)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}

	if (memcpy_verw_s(&ms->ms_n, sizeof(ms->ms_n), &n, sizeof(n))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_flags, sizeof(ms->ms_flags), &flags, sizeof(flags))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (ip != NULL) {
		if (memcpy_verw_s(&ms->ms_ip, sizeof(const char*), &__tmp, sizeof(const char*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_ip % sizeof(*ip) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, ip, _len_ip)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_ip);
		ocalloc_size -= _len_ip;
	} else {
		ms->ms_ip = NULL;
	}

	if (memcpy_verw_s(&ms->ms_iplen, sizeof(ms->ms_iplen), &iplen, sizeof(iplen))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_port, sizeof(ms->ms_port), &port, sizeof(port))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall_switchless(21, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_recvfrom(ssize_t* retval, int fd, void* buf, size_t n, int flags, char* ip, size_t iplen, int* port)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_buf = n;
	size_t _len_ip = iplen;
	size_t _len_port = sizeof(int);

	ms_u_recvfrom_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_recvfrom_t);
	void *__tmp = NULL;

	void *__tmp_buf = NULL;
	void *__tmp_ip = NULL;
	void *__tmp_port = NULL;

	CHECK_ENCLAVE_POINTER(buf, _len_buf);
	CHECK_ENCLAVE_POINTER(ip, _len_ip);
	CHECK_ENCLAVE_POINTER(port, _len_port);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (buf != NULL) ? _len_buf : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (ip != NULL) ? _len_ip : 0))
		return SGX_ERROR_INVALID_PARAMETER;
	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (port != NULL) ? _len_port : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_recvfrom_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_recvfrom_t));
	ocalloc_size -= sizeof(ms_u_recvfrom_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (buf != NULL) {
		if (memcpy_verw_s(&ms->ms_buf, sizeof(void*), &__tmp, sizeof(void*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_buf = __tmp;
		memset_verw(__tmp_buf, 0, _len_buf);
		__tmp = (void *)((size_t)__tmp + _len_buf);
		ocalloc_size -= _len_buf;
	} else {
		ms->ms_buf = NULL;
	}

	if (memcpy_verw_s(&ms->ms_n, sizeof(ms->ms_n), &n, sizeof(n))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_flags, sizeof(ms->ms_flags), &flags, sizeof(flags))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (ip != NULL) {
		if (memcpy_verw_s(&ms->ms_ip, sizeof(char*), &__tmp, sizeof(char*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_ip = __tmp;
		if (_len_ip % sizeof(*ip) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset_verw(__tmp_ip, 0, _len_ip);
		__tmp = (void *)((size_t)__tmp + _len_ip);
		ocalloc_size -= _len_ip;
	} else {
		ms->ms_ip = NULL;
	}

	if (memcpy_verw_s(&ms->ms_iplen, sizeof(ms->ms_iplen), &iplen, sizeof(iplen))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (port != NULL) {
		if (memcpy_verw_s(&ms->ms_port, sizeof(int*), &__tmp, sizeof(int*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp_port = __tmp;
		if (_len_port % sizeof(*port) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		memset_verw(__tmp_port, 0, _len_port);
		__tmp = (void *)((size_t)__tmp + _len_port);
		ocalloc_size -= _len_port;
	} else {
		ms->ms_port = NULL;
	}

	status = sgx_ocall_switchless(22, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (buf) {
			if (memcpy_s((void*)buf, _len_buf, __tmp_buf, _len_buf)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (ip) {
			if (memcpy_s((void*)ip, _len_ip, __tmp_ip, _len_ip)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (port) {
			if (memcpy_s((void*)port, _len_port, __tmp_port, _len_port)) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_setsockopt(int* retval, int fd, int level, int optname, const void* optval, socklen_t optlen)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_optval = optlen;

	ms_u_setsockopt_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_setsockopt_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(optval, _len_optval);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (optval != NULL) ? _len_optval : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_setsockopt_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_setsockopt_t));
	ocalloc_size -= sizeof(ms_u_setsockopt_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_level, sizeof(ms->ms_level), &level, sizeof(level))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (memcpy_verw_s(&ms->ms_optname, sizeof(ms->ms_optname), &optname, sizeof(optname))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	if (optval != NULL) {
		if (memcpy_verw_s(&ms->ms_optval, sizeof(const void*), &__tmp, sizeof(const void*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, optval, _len_optval)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_optval);
		ocalloc_size -= _len_optval;
	} else {
		ms->ms_optval = NULL;
	}

	if (memcpy_verw_s(&ms->ms_optlen, sizeof(ms->ms_optlen), &optlen, sizeof(optlen))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall(23, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL u_close(int* retval, int fd)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_u_close_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_u_close_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_u_close_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_u_close_t));
	ocalloc_size -= sizeof(ms_u_close_t);

	if (memcpy_verw_s(&ms->ms_fd, sizeof(ms->ms_fd), &fd, sizeof(fd))) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}

	status = sgx_ocall_switchless(24, ms);

	if (status == SGX_SUCCESS) {
		if (retval) {
			if (memcpy_s((void*)retval, sizeof(*retval), &ms->ms_retval, sizeof(ms->ms_retval))) {
				sgx_ocfree();
				return SGX_ERROR_UNEXPECTED;
			}
		}
		if (memcpy_s((void*)&errno, sizeof(errno), &ms->ocall_errno, sizeof(ms->ocall_errno))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
	}
	sgx_ocfree();
	return status;
}

