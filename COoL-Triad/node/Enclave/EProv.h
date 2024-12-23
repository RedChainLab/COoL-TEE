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


#ifndef _EPROV_H_
#define _EPROV_H_

#include "ENode.h"
#include <sgx_thread.h>
#include <queue>

struct Request
{
    uint64_t id;
    const char* ip;
    uint16_t port;
    timespec start_ts;
};

typedef enum trusted_time
{
    NO_TIME,
    UNTRUSTED_TIME,
    TRUSTED_TIME
}trusted_time_t;

class EProv
{
    private:
    sgx_thread_rwlock_t stop_rwlock;
    sgx_thread_rwlock_t socket_rwlock;
    sgx_thread_mutex_t rq_mutex;
    sgx_thread_cond_t rq_cond;

    std::queue<Request*> rq_queue;
    int service_time_us;

    int prov_port;
    int sock;
    ENode* node;

    bool stop;
    bool readfrom_stopped;
    bool handle_rq_stopped;
    
    int verbosity;
    trusted_time_t use_time;

    public:
    EProv(int prov_port, int service_time, int triad_port);
    ~EProv();
    int loop_recvfrom();
    int loop_handle_rq();
    void stop_tasks();

    int handle_message(const void* buff, size_t buff_len, char* ip, uint16_t port);
    int sendMessage(const void* buff, size_t buff_len, const char* ip, uint16_t port);
    int setup_socket();

    void eprintf(const char *fmt, ...);
    bool should_stop();
};


#endif /* !_EPROV_H_ */
