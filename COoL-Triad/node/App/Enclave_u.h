#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */

#include "sys/socket.h"
#include "sys/select.h"
#include "netdb.h"
#include "poll.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_PRINT_STRING_DEFINED__
#define OCALL_PRINT_STRING_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_string, (const char* str));
#endif
#ifndef OCALL_READTSC_DEFINED__
#define OCALL_READTSC_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_readTSC, (long long* timestamps));
#endif
#ifndef OCALL_SLEEP_DEFINED__
#define OCALL_SLEEP_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_sleep, (int time));
#endif
#ifndef OCALL_USLEEP_DEFINED__
#define OCALL_USLEEP_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_usleep, (int time));
#endif
#ifndef OCALL_TIMESPEC_GET_DEFINED__
#define OCALL_TIMESPEC_GET_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_timespec_get, (struct timespec* ts));
#endif
#ifndef OCALL_TIMESPEC_PRINT_DEFINED__
#define OCALL_TIMESPEC_PRINT_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_timespec_print, (struct timespec* ts));
#endif
#ifndef SGX_OC_CPUIDEX_DEFINED__
#define SGX_OC_CPUIDEX_DEFINED__
void SGX_UBRIDGE(SGX_CDECL, sgx_oc_cpuidex, (int cpuinfo[4], int leaf, int subleaf));
#endif
#ifndef SGX_THREAD_WAIT_UNTRUSTED_EVENT_OCALL_DEFINED__
#define SGX_THREAD_WAIT_UNTRUSTED_EVENT_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_wait_untrusted_event_ocall, (const void* self));
#endif
#ifndef SGX_THREAD_SET_UNTRUSTED_EVENT_OCALL_DEFINED__
#define SGX_THREAD_SET_UNTRUSTED_EVENT_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_set_untrusted_event_ocall, (const void* waiter));
#endif
#ifndef SGX_THREAD_SETWAIT_UNTRUSTED_EVENTS_OCALL_DEFINED__
#define SGX_THREAD_SETWAIT_UNTRUSTED_EVENTS_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_setwait_untrusted_events_ocall, (const void* waiter, const void* self));
#endif
#ifndef SGX_THREAD_SET_MULTIPLE_UNTRUSTED_EVENTS_OCALL_DEFINED__
#define SGX_THREAD_SET_MULTIPLE_UNTRUSTED_EVENTS_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, sgx_thread_set_multiple_untrusted_events_ocall, (const void** waiters, size_t total));
#endif
#ifndef PTHREAD_WAIT_TIMEOUT_OCALL_DEFINED__
#define PTHREAD_WAIT_TIMEOUT_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, pthread_wait_timeout_ocall, (unsigned long long waiter, unsigned long long timeout));
#endif
#ifndef PTHREAD_CREATE_OCALL_DEFINED__
#define PTHREAD_CREATE_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, pthread_create_ocall, (unsigned long long self));
#endif
#ifndef PTHREAD_WAKEUP_OCALL_DEFINED__
#define PTHREAD_WAKEUP_OCALL_DEFINED__
int SGX_UBRIDGE(SGX_CDECL, pthread_wakeup_ocall, (unsigned long long waiter));
#endif
#ifndef U_SOCKET_DEFINED__
#define U_SOCKET_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, u_socket, (int domain, int type, int protocol));
#endif
#ifndef U_BIND_DEFINED__
#define U_BIND_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, u_bind, (int fd, const struct sockaddr* addr, socklen_t len));
#endif
#ifndef U_LISTEN_DEFINED__
#define U_LISTEN_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, u_listen, (int fd, int n));
#endif
#ifndef U_ACCEPT_DEFINED__
#define U_ACCEPT_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, u_accept, (int fd, struct sockaddr* addr, socklen_t addrlen_in, socklen_t* addrlen_out));
#endif
#ifndef U_CONNECT_DEFINED__
#define U_CONNECT_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, u_connect, (int fd, const struct sockaddr* addr, socklen_t len));
#endif
#ifndef U_SEND_DEFINED__
#define U_SEND_DEFINED__
ssize_t SGX_UBRIDGE(SGX_NOCONVENTION, u_send, (int fd, const void* buf, size_t n, int flags));
#endif
#ifndef U_RECV_DEFINED__
#define U_RECV_DEFINED__
ssize_t SGX_UBRIDGE(SGX_NOCONVENTION, u_recv, (int fd, void* buf, size_t n, int flags));
#endif
#ifndef U_SENDTO_DEFINED__
#define U_SENDTO_DEFINED__
ssize_t SGX_UBRIDGE(SGX_NOCONVENTION, u_sendto, (int fd, const void* buf, size_t n, int flags, const char* ip, size_t iplen, int port));
#endif
#ifndef U_RECVFROM_DEFINED__
#define U_RECVFROM_DEFINED__
ssize_t SGX_UBRIDGE(SGX_NOCONVENTION, u_recvfrom, (int fd, void* buf, size_t n, int flags, char* ip, size_t iplen, int* port));
#endif
#ifndef U_SETSOCKOPT_DEFINED__
#define U_SETSOCKOPT_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, u_setsockopt, (int fd, int level, int optname, const void* optval, socklen_t optlen));
#endif
#ifndef U_CLOSE_DEFINED__
#define U_CLOSE_DEFINED__
int SGX_UBRIDGE(SGX_NOCONVENTION, u_close, (int fd));
#endif

sgx_status_t ecall_init(sgx_enclave_id_t eid, int* retval, uint16_t port);
sgx_status_t ecall_destroy(sgx_enclave_id_t eid, int* retval, uint16_t port);
sgx_status_t ecall_stop(sgx_enclave_id_t eid, int* retval, uint16_t port);
sgx_status_t ecall_monitor(sgx_enclave_id_t eid, int* retval, uint16_t port);
sgx_status_t ecall_loop_recvfrom(sgx_enclave_id_t eid, int* retval, uint16_t port);
sgx_status_t ecall_refresh(sgx_enclave_id_t eid, int* retval, uint16_t port);
sgx_status_t ecall_untaint_trigger(sgx_enclave_id_t eid, int* retval, uint16_t port);
sgx_status_t ecall_add_sibling(sgx_enclave_id_t eid, int* retval, uint16_t port, const char* sibling_ip, uint16_t sibling_port);
sgx_status_t ecall_get_timestamp(sgx_enclave_id_t eid, int* retval, uint16_t port, struct timespec* timestamp);
sgx_status_t ecall_init_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port, int _service_time, uint16_t _triad_port);
sgx_status_t ecall_destroy_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port);
sgx_status_t ecall_stop_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port);
sgx_status_t ecall_loop_recvfrom_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port);
sgx_status_t ecall_loop_handle_rq_prov(sgx_enclave_id_t eid, int* retval, uint16_t _prov_port);
sgx_status_t sl_init_switchless(sgx_enclave_id_t eid, sgx_status_t* retval, void* sl_data);
sgx_status_t sl_run_switchless_tworker(sgx_enclave_id_t eid, sgx_status_t* retval);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
