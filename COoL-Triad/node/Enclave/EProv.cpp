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
#include "sys/socket.h"
#include "Enclave_t.h"
#include "EProv.h"
#include <stdio.h>
#include <sgx_trts_aex.h>
#include <sgx_thread.h>
#include <map>
#include <bitset>

#include "json.hpp"
using namespace json;

extern std::map<int /*port*/, ENode*> nodes;
std::map<int /*port*/, EProv*> provs;

#define RQ_STR "RQ"
#define RSP_STR "RSP"

inline long long int rdtscp(void){
    unsigned int lo, hi;
    __asm__ __volatile__("rdtscp" : "=a" (lo), "=d" (hi));
    //t_print("lo: %d, hi: %d\n", lo, hi);
    return ((uint64_t)hi << 32) | lo;
}

EProv::EProv(int aprov_port, int aservice_time, int triad_port):
    service_time_us(aservice_time), prov_port(aprov_port), sock(-1), 
    node(nodes[triad_port]), stop(false), readfrom_stopped(false), handle_rq_stopped(false), 
    verbosity(0), use_time(TRUSTED_TIME)
{
    eprintf("Creating EProv instance...\r\n");
    sgx_thread_rwlock_init(&stop_rwlock, NULL);
    sgx_thread_mutex_init(&rq_mutex, NULL);
    setup_socket();
    eprintf("EProv instance created.\r\n");
}

EProv::~EProv()
{
    sgx_thread_rwlock_destroy(&stop_rwlock);
    sgx_thread_mutex_destroy(&rq_mutex);
}

int EProv::setup_socket()
{
    if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        eprintf("server socket creation error...\r\n");
        return SOCKET_CREATION_ERROR;
    }

    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons((int16_t)prov_port);
    serAddr.sin_addr.s_addr = INADDR_ANY;

    if ((bind(this->sock, (struct sockaddr*)&serAddr, sizeof(serAddr))) < 0) {
        eprintf("server socket binding error...: %d\r\n", errno);
        close(this->sock);
        return SOCKET_BINDING_ERROR;
    }

    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 200000;
    eprintf("server socket created...: %d\r\n", this->sock);
    return SUCCESS;
}

int EProv::loop_recvfrom()
{
    int retval=SUCCESS;
    while (retval == SUCCESS && !should_stop())
    {    
        char buff[1024] = {0};
        char ip[INET_ADDRSTRLEN];
        int cport;
        if(sock < 0)
        {
            return SOCKET_INEXISTENT;
        }
        ssize_t readStatus = recvfrom(sock, buff, sizeof(buff), 0, ip, INET_ADDRSTRLEN, &cport);
        if (readStatus < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                //eprintf("waiting for data...: %d\r\n", errno);
                continue;
            } else {
                eprintf("reading error...: %d\r\n", errno);
                close(sock);
                sock = -1;
                return READING_ERROR;
            }
        }
        else if(readStatus > 0)
        {
            if(verbosity>=2) eprintf("loop_recvfrom: Message received from %s:%d if len %d: %s\r\n", ip, cport, readStatus, buff);
            retval=handle_message(buff, sizeof(buff), ip, (uint16_t)cport);
        }
    }
    readfrom_stopped = true;
    eprintf("ENode listen stopped.\r\n");
    return retval;
}

int EProv::sendMessage(const void* buff, size_t buff_len, const char* ip, uint16_t cport)
{
    if(sock < 0)
    {
        return SOCKET_INEXISTENT;
    }
    ssize_t sendStatus = sendto(sock, buff, buff_len, 0, ip, INET_ADDRSTRLEN, cport);
    if (sendStatus < 0) {
        eprintf("sending error on socket %d...: %d\r\n", sock, errno);
        close(sock);
        sock = -1;
        return SENDING_ERROR;
    }
    return SUCCESS;
}

int EProv::handle_message(const void* buff, size_t buff_len, char* ip, uint16_t cport)
{
    int retval = SUCCESS;
    if(buff_len == 0)
    {
        eprintf("Empty message received from %s:%d\r\n", ip, cport);
        return retval;
    }
    std::string response((const char*)buff);
    JSON jresp = JSON::Load(response);
    Request* rq = new Request();
    rq->id = jresp["ID"].ToInt();
    rq->ip = ip;
    rq->port = cport;
    switch (use_time)
    {
        case TRUSTED_TIME:
            rq->start_ts = node->get_timestamp();
            break;
        case UNTRUSTED_TIME:
            ocall_timespec_get(&(rq->start_ts));
            break;
        default:
            break;
    }
    sgx_thread_mutex_lock(&rq_mutex);
    rq_queue.push(rq);
    sgx_thread_mutex_unlock(&rq_mutex);
    sgx_thread_cond_signal(&rq_cond);
    return retval;
}

struct timespec diff_timespec(const struct timespec *time1,
    const struct timespec *time0) {
    assert(time1);
    assert(time0);
    struct timespec diff = {.tv_sec = time1->tv_sec - time0->tv_sec,
        .tv_nsec = time1->tv_nsec - time0->tv_nsec};
    if (diff.tv_nsec < 0) {
        diff.tv_nsec += 1000000000;
        diff.tv_sec--;
    }
    return diff;
}

int EProv::loop_handle_rq()
{
    int retval = 0;
    while(!should_stop())
    {
        Request* rq = nullptr;
        sgx_thread_mutex_lock(&rq_mutex);
        if(rq_queue.empty())
        {
            sgx_thread_mutex_unlock(&rq_mutex);
            sgx_thread_cond_wait(&rq_cond, &rq_mutex);
        }
        if(!rq_queue.empty())
        {
            rq = rq_queue.front();
            rq_queue.pop();
        }
        sgx_thread_mutex_unlock(&rq_mutex);
        if(rq)
        {
            //eprintf("Getting TSC.\r\n");
            long long int rd_queue=rdtscp();
            timespec end_ts;
            timespec delay;
            switch(use_time)
            {
                case TRUSTED_TIME:
                    end_ts = node->get_timestamp();
                    delay=diff_timespec(&end_ts, &(rq->start_ts));
                    break;
                case UNTRUSTED_TIME:
                    ocall_timespec_get(&end_ts);
                    delay=diff_timespec(&end_ts, &(rq->start_ts));
                    break;
                default:
                    break;
            }
            long long int rd_start=rdtscp();
            long long int add_count=0;
            long long int start_tsc=rdtscp();
            long long int stop_tsc=3000*service_time_us+start_tsc;
            asm volatile(
                "movq %0, %%r8\n\t"
                "movq %1, %%r9\n\t"
                "movq $0, %%r10\n\t"

                "1: rdtscp\n\t"
                "shlq $32, %%rdx\n\t"
                "orq %%rax, %%rdx\n\t"
                "incq %%r10\n\t"
                "movq %%r10, (%%r8)\n\t"
                "cmpq %%r9, %%rdx\n\t"
                "jl 1b\n\t"
                :
                : "r"(&add_count), "r"(stop_tsc)
                : "rax", "rdx", "r8", "r9"
            );
            long long int rd_served=rdtscp();
            JSON response;
            response["ID"] = rq->id;
            response["delay_s"] = delay.tv_sec;
            response["delay_ns"] = delay.tv_nsec;
            long long int rd_resp=rdtscp();
            retval=sendMessage(response.dump().c_str(), response.dump().length(), rq->ip, rq->port);
            long long int rd_end=rdtscp();
            eprintf("Request %llu handled in %lld TSC.\r\n", rq->id, rd_end-rd_queue);
            eprintf("Intermediate delays: %lld, %lld, %lld, %lld.\r\n", rd_start-rd_queue, rd_served-rd_start, rd_resp-rd_served, rd_end-rd_resp);
            delete rq;
        }
    }
    handle_rq_stopped = true;
    return retval;
}

void EProv::eprintf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    std::string str = std::string("[EProv ") + std::to_string(prov_port) + "]> ";
    str += fmt;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, BUFSIZ, str.c_str(), args);
    va_end(args);
    ocall_print_string(buf);
}

void EProv::stop_tasks()
{
    eprintf("Stopping EProv instance...\r\n");
    eprintf("Sending stop...\r\n");
    sgx_thread_rwlock_wrlock(&stop_rwlock);
    stop=true;
    sgx_thread_rwlock_unlock(&stop_rwlock);
    sgx_thread_cond_signal(&rq_cond);
    eprintf("Waiting for handle_rq and readfrom tasks...\r\n");
    while(!readfrom_stopped && !handle_rq_stopped);
    eprintf("Handle_rq and readfrom tasks stopped.\r\n");
    eprintf("Closing socket...\r\n");
    sgx_thread_rwlock_wrlock(&socket_rwlock);
    close(sock);
    sock = -1;
    sgx_thread_rwlock_unlock(&socket_rwlock);
    eprintf("EProv tasks stopped.\r\n");
}

bool EProv::should_stop()
{
    sgx_thread_rwlock_rdlock(&stop_rwlock);
    bool retval = stop;
    sgx_thread_rwlock_unlock(&stop_rwlock);
    return retval;
}