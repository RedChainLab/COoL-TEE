/*
 * Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifndef _ENCLAVE_H_
#define _ENCLAVE_H_

#include <stdlib.h>
#include <assert.h>

#include "sodium.h"
#include <sgx_thread.h>
#include <vector>
#include <string>

#if defined(__cplusplus)
extern "C" {
#endif

int printf(const char *fmt, ...);

#if defined(__cplusplus)
}
#endif

#define SIZE 1024

enum {
    SUCCESS = 0,
    SOCKET_ALREADY_EXISTS = -1,
    SOCKET_CREATION_ERROR = -2,
    SOCKET_BINDING_ERROR = -3,
    READING_ERROR = -4,
    DECRYPTION_FAILED = -5,
    SENDING_ERROR = -6,
    SOCKET_INEXISTENT = -7
}; 

typedef struct {
    int port;
    long long int* add_count;
    long long int* mem_add_count;

    long long int* total_aex_count;

    bool* tainted;
    sgx_thread_mutex_t* tainted_mutex;
    sgx_thread_cond_t* tainted_cond;

    long long int* aex_count;
    long long int* monitor_aex_count;

    long long int* count_aex;
    long long int* monitor_aex;
}aex_handler_args_t;

typedef struct {
    long long int msg_id;
    long long int total_aex_count;
    long long int tsc;
}calib_msg_t;

class ENode
{
public:
    int port;
    bool stop;

    long long int add_count;
    long long int total_aex_count;

    bool calib_count;
    bool calib_ts_ref;
    bool tainted;
    long long int mem_add_count;

    long long int aex_count;
    long long int monitor_aex_count;

    long long int count_aex[SIZE];
    long long int monitor_aex[SIZE];

    sgx_thread_rwlock_t stop_rwlock;
    sgx_thread_rwlock_t socket_rwlock;

    sgx_thread_mutex_t tainted_mutex;
    sgx_thread_mutex_t calib_mutex;
    sgx_thread_cond_t tainted_cond;
    sgx_thread_cond_t untainted_cond;

    aex_handler_args_t aex_args;

    void monitor();
    int loop_recvfrom();
    void refresh();
    void untaint_trigger();
    void stop_tasks();

    int add_sibling(std::string hostname, uint16_t port);

    timespec get_timestamp();

    ENode(int _port);
    ~ENode();

private:
    int sock;

    std::pair<std::string, uint16_t> time_authority;

    int sleep_time;
    int verbosity;

    long long tsc;
    double tsc_freq;

    long long tsc_ref;
    timespec ts_ref;
    timespec ts_curr;

    long long int calib_msg_count;
    static const int NB_CALIB_MSG = 10;
    calib_msg_t calib_sent[NB_CALIB_MSG];
    calib_msg_t calib_recvd[NB_CALIB_MSG];

    long long add_count_ref;

    unsigned char nonce[crypto_aead_aes256gcm_NPUBBYTES];
    unsigned char key[crypto_aead_aes256gcm_KEYBYTES];

    std::vector<std::pair<std::string, uint16_t>> siblings;

    bool monitor_stopped;
    bool refresh_stopped;
    bool trigger_stopped;
    bool readfrom_stopped;

    bool should_stop();

    int setup_socket();

    bool calibrate();
    bool calibrate_drift();
    bool calibrate_count();
    bool monitor_rdtsc();

    int handle_message(const void* buff, size_t buff_len, char* ip, uint16_t port);
    int sendMessage(const void* buff, size_t buff_len, const char* ip, uint16_t port);

    void send_recv_drift_message(int sleep_time_ms, int sleep_attack_ms=0);

    void incrementNonce();
    int encrypt(const unsigned char* plaintext, const unsigned long long plen, unsigned char* ciphertext, unsigned long long* clen);
    int decrypt(const unsigned char* ciphertext, const unsigned long long clen, unsigned char* decrypted, unsigned long long* dlen);
    int test_encdec();

    void eprintf(const char *fmt, ...);

    void print_siblings();
};

#endif /* !_ENCLAVE_H_ */
