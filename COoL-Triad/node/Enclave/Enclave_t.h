#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */

#include "sys/socket.h"
#include "sys/select.h"
#include "netdb.h"
#include "poll.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

int ecall_init(uint16_t port);
int ecall_destroy(uint16_t port);
int ecall_stop(uint16_t port);
int ecall_monitor(uint16_t port);
int ecall_loop_recvfrom(uint16_t port);
int ecall_refresh(uint16_t port);
int ecall_untaint_trigger(uint16_t port);
int ecall_add_sibling(uint16_t port, const char* sibling_ip, uint16_t sibling_port);
int ecall_get_timestamp(uint16_t port, struct timespec* timestamp);
int ecall_init_prov(uint16_t _prov_port, int _service_time, uint16_t _triad_port);
int ecall_destroy_prov(uint16_t _prov_port);
int ecall_stop_prov(uint16_t _prov_port);
int ecall_loop_recvfrom_prov(uint16_t _prov_port);
int ecall_loop_handle_rq_prov(uint16_t _prov_port);
sgx_status_t sl_init_switchless(void* sl_data);
sgx_status_t sl_run_switchless_tworker(void);

sgx_status_t SGX_CDECL ocall_print_string(const char* str);
sgx_status_t SGX_CDECL ocall_readTSC(long long* timestamps);
sgx_status_t SGX_CDECL ocall_sleep(int time);
sgx_status_t SGX_CDECL ocall_usleep(int time);
sgx_status_t SGX_CDECL ocall_timespec_get(struct timespec* ts);
sgx_status_t SGX_CDECL ocall_timespec_print(struct timespec* ts);
sgx_status_t SGX_CDECL sgx_oc_cpuidex(int cpuinfo[4], int leaf, int subleaf);
sgx_status_t SGX_CDECL sgx_thread_wait_untrusted_event_ocall(int* retval, const void* self);
sgx_status_t SGX_CDECL sgx_thread_set_untrusted_event_ocall(int* retval, const void* waiter);
sgx_status_t SGX_CDECL sgx_thread_setwait_untrusted_events_ocall(int* retval, const void* waiter, const void* self);
sgx_status_t SGX_CDECL sgx_thread_set_multiple_untrusted_events_ocall(int* retval, const void** waiters, size_t total);
sgx_status_t SGX_CDECL pthread_wait_timeout_ocall(int* retval, unsigned long long waiter, unsigned long long timeout);
sgx_status_t SGX_CDECL pthread_create_ocall(int* retval, unsigned long long self);
sgx_status_t SGX_CDECL pthread_wakeup_ocall(int* retval, unsigned long long waiter);
sgx_status_t SGX_CDECL u_socket(int* retval, int domain, int type, int protocol);
sgx_status_t SGX_CDECL u_bind(int* retval, int fd, const struct sockaddr* addr, socklen_t len);
sgx_status_t SGX_CDECL u_listen(int* retval, int fd, int n);
sgx_status_t SGX_CDECL u_accept(int* retval, int fd, struct sockaddr* addr, socklen_t addrlen_in, socklen_t* addrlen_out);
sgx_status_t SGX_CDECL u_connect(int* retval, int fd, const struct sockaddr* addr, socklen_t len);
sgx_status_t SGX_CDECL u_send(ssize_t* retval, int fd, const void* buf, size_t n, int flags);
sgx_status_t SGX_CDECL u_recv(ssize_t* retval, int fd, void* buf, size_t n, int flags);
sgx_status_t SGX_CDECL u_sendto(ssize_t* retval, int fd, const void* buf, size_t n, int flags, const char* ip, size_t iplen, int port);
sgx_status_t SGX_CDECL u_recvfrom(ssize_t* retval, int fd, void* buf, size_t n, int flags, char* ip, size_t iplen, int* port);
sgx_status_t SGX_CDECL u_setsockopt(int* retval, int fd, int level, int optname, const void* optval, socklen_t optlen);
sgx_status_t SGX_CDECL u_close(int* retval, int fd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
