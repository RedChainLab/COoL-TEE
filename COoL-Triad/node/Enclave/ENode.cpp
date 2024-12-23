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
#include "ENode.h"
#include <stdio.h>
#include <sgx_trts_aex.h>
#include <sgx_thread.h>
#include <map>

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, BUFSIZ, fmt, args);
    va_end(args);
    ocall_print_string(buf);
    return 0;
}

#ifdef __cplusplus
}
#endif

#define TAINTED_STR "Tainted"
#define UNTAINTING_STR "Untaint"
#define DRIFT_STR "Drift"

std::map<int /*port*/, ENode*> nodes;

static void printArray(long long int *arr, long long int size, long long int reference){
    /*
    Print a array of size SIZE, which contains the number of ADD operations performed before each AEX occurs.
    */
    for(int i = 0; i < size ; i++){
        printf("%d;%lld\n", i, arr[i]-reference);
    }
}

inline void log_aex(long long int* arr, long long int& next_index, long long int* add_count){
    if(next_index < SIZE)
    {
        arr[next_index++] = *add_count;
    }
    else
    {
        printf("Error: Array is full\n");
    }
}

static void aex_handler(const sgx_exception_info_t *info, const void * args)
{
    (void)info;
    const aex_handler_args_t* aex_args=(const aex_handler_args_t*)args;
    *aex_args->tainted = true;
    *aex_args->mem_add_count = *aex_args->add_count;
    *aex_args->total_aex_count += 1;
    sgx_thread_mutex_lock(aex_args->tainted_mutex);
    printf("[Handler %d]> AEX %lld\r\n", aex_args->port, *aex_args->total_aex_count);
    sgx_thread_cond_signal(aex_args->tainted_cond);
    sgx_thread_mutex_unlock(aex_args->tainted_mutex);

    log_aex(aex_args->count_aex, *(aex_args->aex_count), aex_args->add_count);
}

inline long long int rdtscp(void){
    unsigned int lo, hi;
    __asm__ __volatile__("rdtscp" : "=a" (lo), "=d" (hi));
    //t_print("lo: %d, hi: %d\n", lo, hi);
    return ((uint64_t)hi << 32) | lo;
}

void ENode::untaint_trigger()
{
    while(!should_stop())
    {
        sgx_thread_mutex_lock(&tainted_mutex);
        if(tainted)
        {
            sgx_thread_cond_signal(&tainted_cond);
        }
        sgx_thread_mutex_unlock(&tainted_mutex);
        ocall_sleep(1);
    }
    trigger_stopped = true;
    eprintf("Untainting trigger stopped.\r\n");
}

void ENode::refresh()
{
    while(!should_stop())
    {
        sgx_thread_mutex_lock(&tainted_mutex);
        sgx_thread_cond_wait(&tainted_cond, &tainted_mutex);
        if(should_stop())
        {
            eprintf("Stopping refresh.\r\n");
            sgx_thread_mutex_unlock(&tainted_mutex);
            break;
        }
        else if(tainted && calib_count && calib_ts_ref)
        {
            sgx_thread_mutex_unlock(&tainted_mutex);
            eprintf("Untainting\r\n");
            long long int mem_total_aex_count;
            do
            {
                mem_total_aex_count=total_aex_count;
                for(long unsigned int i=0; i < siblings.size() && tainted; i++)
                {
                    eprintf("Sending untaint to %s:%d\r\n", siblings[i].first.c_str(), siblings[i].second);
                    sendMessage(TAINTED_STR, strlen(TAINTED_STR), siblings[i].first.c_str(), siblings[i].second);
                }
                ocall_usleep(100000);
            } while (mem_total_aex_count!=total_aex_count);
            if(tainted)
            {
                eprintf("Peer untainting failed.\r\n");
                calib_ts_ref = false;
            }
        }
        else
        {
            eprintf("State (tainted, count, ts_ref)=(%d, %d, %d) not ready.\r\n", tainted, calib_count, calib_ts_ref);
            sgx_thread_mutex_unlock(&tainted_mutex);
        }
    }
    refresh_stopped = true;
    eprintf("Refresh stopped.\r\n");
}

ENode::ENode(int _port):port(_port), stop(false), 
    add_count(0), total_aex_count(0), calib_count(false), calib_ts_ref(false),
    tainted(true), aex_count(0), monitor_aex_count(0), 
    sock(-1), time_authority(std::make_pair("127.0.0.1",12340)),sleep_time(500), verbosity(0), 
    monitor_stopped(false), refresh_stopped(false), trigger_stopped(false)
{
    eprintf("Creating ENode instance...\r\n");
    memset(count_aex, 0, sizeof(count_aex));
    memset(monitor_aex, 0, sizeof(monitor_aex));

    aex_args.port = port;

    aex_args.add_count = &add_count;
    aex_args.mem_add_count = &mem_add_count;

    aex_args.total_aex_count = &total_aex_count;

    aex_args.tainted = &tainted;
    aex_args.tainted_mutex = &tainted_mutex;
    aex_args.tainted_cond = &tainted_cond;

    aex_args.aex_count = &aex_count;
    aex_args.monitor_aex_count = &monitor_aex_count;
    aex_args.count_aex = count_aex;
    aex_args.monitor_aex = monitor_aex;
    
    incrementNonce();
    //randombytes_buf(key, sizeof(key));
    const char* test_key = "b52c505a37d78eda5dd34f20c22540ea1b58963cf8e5bf8ffa85f9f2492505b4";
    sodium_hex2bin(key, crypto_aead_aes256gcm_KEYBYTES,
                       test_key, strlen(test_key),
                       NULL, NULL, NULL);

    sgx_thread_mutex_init(&tainted_mutex, NULL);
    sgx_thread_rwlock_init(&stop_rwlock, NULL);
    sgx_thread_rwlock_init(&socket_rwlock, NULL);
    sgx_thread_mutex_init(&calib_mutex, NULL);
    setup_socket();
    test_encdec();
    eprintf("ENode instance created.\r\n");
}

void ENode::stop_tasks()
{
    eprintf("Stopping ENode instance...\r\n");
    print_siblings();
    eprintf("Sending stop...\r\n");
    sgx_thread_rwlock_wrlock(&stop_rwlock);
    stop=true;
    sgx_thread_rwlock_unlock(&stop_rwlock);
    while(!monitor_stopped&&!readfrom_stopped);
    eprintf("Monitor and readfrom tasks stopped.\r\n");
    eprintf("Signalling refresh to stop.\r\n");
    sgx_thread_mutex_lock(&tainted_mutex);
    sgx_thread_cond_signal(&tainted_cond);
    sgx_thread_mutex_unlock(&tainted_mutex);
    while(!refresh_stopped && !trigger_stopped);
    eprintf("Refresh and untaint trigger stopped.\r\n");
    eprintf("Closing socket...\r\n");
    sgx_thread_rwlock_wrlock(&socket_rwlock);
    close(sock);
    sock = -1;
    sgx_thread_rwlock_unlock(&socket_rwlock);
    eprintf("ENode tasks stopped.\r\n");
}

ENode::~ENode()
{
    sgx_thread_mutex_destroy(&tainted_mutex);
    sgx_thread_cond_destroy(&tainted_cond);
    sgx_thread_cond_destroy(&untainted_cond);
    sgx_thread_rwlock_destroy(&stop_rwlock);
    sgx_thread_rwlock_destroy(&socket_rwlock);
    sgx_thread_mutex_destroy(&calib_mutex);
}

void ENode::eprintf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    std::string str = std::string("[ENode ") + std::to_string(port) + "]> ";
    str += fmt;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, BUFSIZ, str.c_str(), args);
    va_end(args);
    ocall_print_string(buf);
}

bool ENode::should_stop()
{
    sgx_thread_rwlock_rdlock(&stop_rwlock);
    bool retval = stop;
    sgx_thread_rwlock_unlock(&stop_rwlock);
    return retval;
}

void ENode::incrementNonce(void)
{
    static bool init = false;
    if(!init)
    {
        memset(nonce, 0, sizeof(nonce));
        init = true;
    }
    else
    {
        /*for(unsigned int i = 0; i < crypto_aead_aes256gcm_NPUBBYTES; i++)
        {
            nonce[i]++;
            if(nonce[i] != 0)
            {
                break;
            }
        }*/
    }
}

int ENode::encrypt(const unsigned char* plaintext, const unsigned long long plen, unsigned char* ciphertext, unsigned long long* clen)
{
    incrementNonce();

    crypto_aead_aes256gcm_encrypt(ciphertext, clen,
                                  plaintext, plen,
                                  NULL, 0, NULL, nonce, key);
    
    return SUCCESS;
}

int ENode::decrypt(const unsigned char* ciphertext, const unsigned long long clen, unsigned char* decrypted, unsigned long long* dlen)
{
    if (crypto_aead_aes256gcm_decrypt(decrypted, dlen,
                                      NULL, ciphertext, clen,
                                      NULL, 0, nonce, key) != 0) {
        eprintf("Decryption failed\r\n");
        return -1;
    }
    return 0;
}

int ENode::test_encdec()
{
    unsigned char msg[] = "Hello!";
    unsigned long long msg_len = sizeof(msg);
    unsigned char ciphertext[sizeof(msg) + crypto_aead_aes256gcm_ABYTES];
    unsigned long long ciphertext_len = sizeof(ciphertext);
    unsigned char decrypted[sizeof(msg)];
    unsigned long long decrypted_len = sizeof(decrypted);
    eprintf("Message: %s\r\n", msg);
    encrypt(msg, msg_len, ciphertext, &ciphertext_len);
    eprintf("Encrypted: %s\r\n", ciphertext);
    if(decrypt(ciphertext, ciphertext_len, decrypted, &decrypted_len))
    {
        return DECRYPTION_FAILED;
    }
    eprintf("Decrypted: %s\r\n", decrypted);
    return SUCCESS;
}

int ENode::setup_socket()
{
    sgx_thread_rwlock_wrlock(&socket_rwlock);

    if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        eprintf("server socket creation error...\r\n");
        return SOCKET_CREATION_ERROR;
    }

    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons((int16_t)port);
    serAddr.sin_addr.s_addr = INADDR_ANY;

    if ((bind(this->sock, (struct sockaddr*)&serAddr, sizeof(serAddr))) < 0) {
        eprintf("server socket binding error...: %d\r\n", errno);
        close(this->sock);
        return SOCKET_BINDING_ERROR;
    }

    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 200000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
    sgx_thread_rwlock_unlock(&socket_rwlock);
    eprintf("server socket created...: %d\r\n", this->sock);
    return SUCCESS;
}

int ENode::loop_recvfrom()
{
    int retval=SUCCESS;
    while (retval == SUCCESS && !should_stop())
    {    
        char buff[1024] = {0};
        char ip[INET_ADDRSTRLEN];
        int cport;

        sgx_thread_rwlock_rdlock(&socket_rwlock);
        if(sock < 0)
        {
            sgx_thread_rwlock_unlock(&socket_rwlock);
            return SOCKET_INEXISTENT;
        }
        ssize_t readStatus = recvfrom(sock, buff, sizeof(buff), 0, ip, INET_ADDRSTRLEN, &cport);
        sgx_thread_rwlock_unlock(&socket_rwlock);
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

            unsigned char buff_dec[sizeof(buff)];
            unsigned long long buff_len_dec = sizeof(buff);
            retval=decrypt((unsigned char*)buff, readStatus, buff_dec, &buff_len_dec);
            if(retval)
            {
                eprintf("loop_recvfrom: Decryption failed.\r\n");
            }
            else
            {
                if(verbosity>=2) eprintf("loop_recvfrom: Decrypted message: %s\r\n", buff_dec);
            }

            retval=handle_message(buff_dec, buff_len_dec, ip, (uint16_t)cport);
        }
    }
    readfrom_stopped = true;
    eprintf("ENode listen stopped.\r\n");
    return retval;
}

int ENode::sendMessage(const void* buff, size_t buff_len, const char* ip, uint16_t cport)
{
    sgx_thread_rwlock_rdlock(&socket_rwlock);
    if(sock < 0)
    {
        sgx_thread_rwlock_unlock(&socket_rwlock);
        return SOCKET_INEXISTENT;
    }
    unsigned char buff_enc[buff_len + crypto_aead_aes256gcm_ABYTES];
    unsigned long long buff_len_enc = buff_len + crypto_aead_aes256gcm_ABYTES;
    int retval=encrypt((const unsigned char*)buff, buff_len, buff_enc, &buff_len_enc);
    if(verbosity>=2) eprintf("sendMessage: Encrypting message to %s:%d of len %d: %s\r\n", ip, cport, buff_len, buff);
    if(retval)
    {
        eprintf("sendMessage: Encryption failed.\r\n");
    }
    else
    {
        if(verbosity>=2) eprintf("sendMessage: Encrypted message of len %d: %s\r\n", buff_len_enc, buff_enc);
    }
    ssize_t sendStatus = sendto(sock, buff_enc, buff_len_enc, 0, ip, INET_ADDRSTRLEN, cport);
    sgx_thread_rwlock_unlock(&socket_rwlock);
    if (sendStatus < 0) {
        eprintf("sending error on socket %d...: %d\r\n", sock, errno);
        sgx_thread_rwlock_wrlock(&socket_rwlock);
        close(sock);
        sock = -1;
        sgx_thread_rwlock_unlock(&socket_rwlock);
        return SENDING_ERROR;
    }
    return SUCCESS;
}

int ENode::handle_message(const void* buff, size_t buff_len, char* ip, uint16_t cport)
{
    int retval = SUCCESS;
    if(buff_len == 0)
    {
        eprintf("Empty message received from %s:%d\r\n", ip, cport);
        return retval;
    }
    switch(((const char*)buff)[0])
    {
        case 'S':
        {
            if(verbosity>=2) eprintf("Sibling message received from %s:%d\r\n", ip, cport);
            if(std::find(siblings.begin(), siblings.end(), std::make_pair(std::string(ip), cport)) == siblings.end())
            {
                siblings.emplace_back(ip, cport);
                if(verbosity>=2) eprintf("Sibling added.\r\n");
            }
            else
            {
                if(verbosity>=2) eprintf("Sibling already added.\r\n");
            }
            break;
        }
        case 'T':
        {
            if(verbosity>=2) eprintf("Tainted message received from %s:%d\r\n", ip, cport);
            if(!tainted)
            {
                timespec timestamp;
                long long total_nsec = (long long)((double)(tsc-tsc_ref)/tsc_freq);
                timestamp.tv_sec = (total_nsec+ts_ref.tv_nsec)/1000000000;
                timestamp.tv_sec += ts_ref.tv_sec;
                timestamp.tv_nsec = (total_nsec+ts_ref.tv_nsec)%1000000000; 
                eprintf("Sending untainting ts: %ld.%ld\r\n", timestamp.tv_sec, timestamp.tv_nsec);
                ocall_timespec_print(&timestamp);
                char send_buff[1024] = {0};
                memcpy(send_buff, UNTAINTING_STR, strlen(UNTAINTING_STR));
                memcpy(send_buff+strlen(UNTAINTING_STR), &timestamp, sizeof(timespec));
                retval=sendMessage(send_buff, strlen(UNTAINTING_STR)+sizeof(timespec), ip, cport);
            }
            break;
        }
        case 'U':
        {
            if(verbosity>=2) eprintf("Untainting message received from %s:%d\r\n", ip, cport);
            sgx_thread_mutex_lock(&tainted_mutex);
            timespec timestamp;
            timestamp = *(const timespec*)((const char*)buff+strlen(UNTAINTING_STR));
            eprintf("Untainting with ts: %ld.%ld\r\n", timestamp.tv_sec, timestamp.tv_nsec);
            ocall_timespec_print(&timestamp);
            long long int mem_tsc=tsc;
            while((mem_tsc==tsc || !calib_count || !calib_ts_ref) & !should_stop());
            timespec curr_ts;
            long long total_nsec = (long long)((double)(tsc-tsc_ref)/tsc_freq);
            tsc_ref=tsc;
            curr_ts.tv_sec = (total_nsec+ts_ref.tv_nsec)/1000000000;
            curr_ts.tv_sec += ts_ref.tv_sec;
            curr_ts.tv_nsec = (total_nsec+ts_ref.tv_nsec)%1000000000; 
            if(timestamp.tv_sec>curr_ts.tv_sec || (timestamp.tv_sec==curr_ts.tv_sec && timestamp.tv_nsec > curr_ts.tv_nsec))
            {
                ts_ref=timestamp;
            }
            else
            {
                ts_ref=curr_ts;
            }
            tainted = false;
            sgx_thread_cond_signal(&untainted_cond);
            sgx_thread_mutex_unlock(&tainted_mutex);
            break;
        }
        case 'D':
        {
            eprintf("Drift message received from %s:%d\r\n", ip, cport);
            eprintf("Msg of len: %d: %s\r\n", buff_len, buff);

            const long long int recvd_calib_msg_count = *(const long long int*)((const char*)buff+strlen(DRIFT_STR));
            const int msg_sleep_time=*(const int*)((const char*)buff+strlen(DRIFT_STR)+sizeof(recvd_calib_msg_count));
            eprintf("Msg contents: %lld %d\r\n", recvd_calib_msg_count, msg_sleep_time);
            sgx_thread_mutex_lock(&calib_mutex);
            calib_recvd[recvd_calib_msg_count%NB_CALIB_MSG]={recvd_calib_msg_count, total_aex_count,tsc};
            sgx_thread_mutex_unlock(&calib_mutex);
            break;
        }
        default:
        {
            retval=sendMessage(buff, buff_len, ip, cport);
            break;
        }
    }
    return retval;
}

void ENode::print_siblings()
{
    eprintf("%d siblings: ", siblings.size());
    for(auto& sibling: siblings)
    {
        printf("%s:%d, ", sibling.first.c_str(), sibling.second);
    }
    printf("\r\n");
}

bool ENode::calibrate()
{
    eprintf("Calibrating...\r\n");
    if(!calib_count)
    {
        calibrate_count();
        eprintf("Calibrating drift...\r\n");
        calibrate_drift();
        eprintf("Measured TSC frequency: %f\r\n", tsc_freq);
        calib_count=true;
    }
    if(!calib_ts_ref)
    {
        ocall_timespec_get(&ts_ref);
        eprintf("Reference time: %ld.%09ld\r\n", ts_ref.tv_sec, ts_ref.tv_nsec);
        ts_curr = ts_ref;
        tsc_ref = rdtscp();
        eprintf("Reference TSC: %lld\r\n", tsc_ref);
        calib_ts_ref=true;
    }
    eprintf("Calibration done.\r\n");
    tainted = false;
    return true;
}

bool ENode::calibrate_count()
{
    int NB_RUNS=10;
    long long int add_count_sum = 0;
    long long int add_count_mem = add_count_sum;
    for(int i=1; i<=NB_RUNS && !should_stop(); i++)
    {
        add_count_mem = add_count_sum;
        monitor_rdtsc();
        add_count_sum += add_count;
        if(add_count_sum < add_count_mem)
        {
            eprintf("Overflow detected!\r\n");
            return false;
        }
        add_count_ref = add_count_sum/i;
    }
    return true;
}

bool ENode::calibrate_drift()
{
    int sleep_time_ms=1000;
    int NB_RUNS=10;
    long long int tsc_tbl[NB_RUNS];
    int nb_ok_runs=0;
    while(nb_ok_runs<NB_RUNS && !should_stop())
    {
        for(int i=0; i<NB_RUNS && !should_stop();i++)
        {
            eprintf("Sending slow drift message %d...\r\n", i+1);
            send_recv_drift_message(sleep_time_ms);
            ocall_usleep(10000);
        }
        eprintf("Updating tsc for 1s...\r\n");
        long long int reference_tsc=rdtscp();
        long long int mem_total_aex_count=total_aex_count;
        for(;(tsc-reference_tsc<3000000000)&&mem_total_aex_count==total_aex_count&&!should_stop();)
        {
            tsc=rdtscp();
        }
        for (int i=0; i<NB_CALIB_MSG && nb_ok_runs<NB_RUNS && !should_stop(); i++)
        {
            eprintf("Checking drift message %d...\r\n", i);
            if(calib_sent[i].msg_id>0 && calib_recvd[i].msg_id==calib_sent[i].msg_id)
            {
                eprintf("Drift message %lld received!\r\n", calib_sent[i].msg_id);
                long long int diff_total_aex_count=calib_recvd[i].total_aex_count-calib_sent[i].total_aex_count;
                if(diff_total_aex_count==0)
                {
                    long long int diff_tsc=calib_recvd[i].tsc-calib_sent[i].tsc;
                    tsc_tbl[nb_ok_runs++]=diff_tsc;
                }
                else if (diff_total_aex_count<0)
                {
                    eprintf("Error: Negative total AEX count difference detected!\r\n");
                }
            }
            else
            {
                eprintf("Error: Drift message %lld not received!\r\n", calib_sent[i].msg_id);
            }
        }
        //sgx_thread_mutex_unlock(&calib_mutex);
        for(int i=0; i<NB_CALIB_MSG;i++)
        {
            eprintf("Sent drift message %lld: %lld %lld\r\n", calib_sent[i].msg_id, calib_sent[i].total_aex_count, calib_sent[i].tsc);
            eprintf("Recvd drift message %lld: %lld %lld\r\n", calib_recvd[i].msg_id, calib_recvd[i].total_aex_count, calib_recvd[i].tsc);
        }
        eprintf("OK runs: %d\r\n", nb_ok_runs);
        memset(calib_sent, 0, sizeof(calib_sent));
        memset(calib_recvd, 0, sizeof(calib_recvd));
    }
    long double avg_tsc_slow_count=0;
    for(int i=0; i<NB_RUNS && !should_stop(); i++)
    {
        long double mem_avg_tsc_count=avg_tsc_slow_count;
        avg_tsc_slow_count+=tsc_tbl[i];
        if(avg_tsc_slow_count<mem_avg_tsc_count)
        {
            eprintf("Overflow detected!\r\n");
            return false;
        }
    }
    avg_tsc_slow_count/=NB_RUNS;
    nb_ok_runs=0;
    while(nb_ok_runs<NB_RUNS && !should_stop())
    {
        for(int i=0; i<NB_RUNS && nb_ok_runs<NB_RUNS && !should_stop();i++)
        {
            eprintf("Sending fast drift message %d...\r\n", i+1);
            send_recv_drift_message(0);
            ocall_usleep(10000);
        }
        eprintf("Updating tsc for 1s...\r\n");
        long long int reference_tsc=rdtscp();
        long long int mem_total_aex_count=total_aex_count;
        for(;(tsc-reference_tsc<3000000000)&&mem_total_aex_count==total_aex_count&&!should_stop();)
        {
            tsc=rdtscp();
        }
        for (int i=0; i<NB_CALIB_MSG && !should_stop(); i++)
        {
            eprintf("Checking drift message %d...\r\n", i);
            if(calib_sent[i].msg_id>0 && calib_recvd[i].msg_id==calib_sent[i].msg_id)
            {
                eprintf("Drift message %lld received!\r\n", calib_sent[i].msg_id);
                long long int diff_total_aex_count=calib_recvd[i].total_aex_count-calib_sent[i].total_aex_count;
                if(diff_total_aex_count==0)
                {
                    long long int diff_tsc=calib_recvd[i].tsc-calib_sent[i].tsc;
                    tsc_tbl[nb_ok_runs++]=diff_tsc;
                }
                else if (diff_total_aex_count<0)
                {
                    eprintf("Error: Negative total AEX count difference detected!\r\n");
                }
            }
            else
            {
                eprintf("Error: Drift message %lld not received!\r\n", calib_sent[i].msg_id);
            }
        }
        memset(calib_sent, 0, sizeof(calib_sent));
        memset(calib_recvd, 0, sizeof(calib_recvd));
    }
    long double avg_tsc_fast_count=0;
    for(int i=0; i<NB_RUNS && !should_stop(); i++)
    {
        long double mem_avg_tsc_count=avg_tsc_fast_count;
        avg_tsc_fast_count+=tsc_tbl[i];
        if(avg_tsc_fast_count<mem_avg_tsc_count)
        {
            eprintf("Overflow detected!\r\n");
            return false;
        }
    }
    avg_tsc_fast_count/=NB_RUNS;
    tsc_freq=(double)(avg_tsc_slow_count-avg_tsc_fast_count)/(1000000*sleep_time_ms);
    return true;
}

void ENode::send_recv_drift_message(int sleep_time_ms, int sleep_attack_ms)
{
    int total_sleep_time_ms=sleep_time_ms+sleep_attack_ms;
    char send_buff[1024] = {0};
    memcpy(send_buff, DRIFT_STR, strlen(DRIFT_STR));
    memcpy(send_buff+strlen(DRIFT_STR), &calib_msg_count, sizeof(calib_msg_count));
    memcpy(send_buff+strlen(DRIFT_STR)+sizeof(calib_msg_count), &total_sleep_time_ms, sizeof(total_sleep_time_ms));

    calib_sent[calib_msg_count%NB_CALIB_MSG]={calib_msg_count, total_aex_count, tsc};
    sendMessage(send_buff, strlen(DRIFT_STR)+sizeof(calib_msg_count)+sizeof(total_sleep_time_ms), time_authority.first.c_str(), time_authority.second);
    calib_msg_count++;
}

bool ENode::monitor_rdtsc()
{
    long long int start_tsc=rdtscp();
    long long int stop_tsc=3000000*sleep_time+start_tsc;
    long long int mem_total_aex_count=total_aex_count;
    asm volatile(
        "movq %0, %%r8\n\t"
        "movq %1, %%r9\n\t"
        "movq $0, %%r10\n\t"
        "movq %2, %%r11\n\t"

        "1: rdtscp\n\t"
        "shlq $32, %%rdx\n\t"
        "orq %%rax, %%rdx\n\t"
        "movq %%rdx, (%%r11)\n\t"
        "incq %%r10\n\t"
        "movq %%r10, (%%r8)\n\t"
        "cmpq %%r9, %%rdx\n\t"
        "jl 1b\n\t"
        :
        : "r"(&add_count), "r"(stop_tsc), "r"(&tsc)
        : "rax", "rdx", "r8", "r9"
    );
    double ACCURACY=0.05;
    if(total_aex_count==mem_total_aex_count && ((double)add_count>(double)add_count_ref*(1+ACCURACY)
    || (double)add_count<(double)add_count_ref*(1-ACCURACY)))
    {
        eprintf("Discalibrated! %f %d %f\r\n",(double)add_count_ref*(1-ACCURACY),add_count,(double)add_count_ref*(1+ACCURACY));
        calib_count = false;
        calib_ts_ref = false;
    }
    return calib_count;
}

void ENode::monitor(){
    sgx_aex_mitigation_node_t node;

    sgx_register_aex_handler(&node, aex_handler, (const void*)&aex_args);
    long long int reference = 0;
    while (!should_stop())
    {    
        if(!calib_count||!calib_ts_ref)
        {
            calibrate();
            continue;
        }
        reference=0;
        monitor_rdtsc();

        if(verbosity>=1)
        {
            eprintf("Monitoring (%s)...\r\n", tainted?"Tainted":"Not tainted");
            printf("idx;count\r\n");
            printArray(count_aex, aex_count, reference);
            printf("%lld;%lld\r\n", aex_count, add_count-reference);
        }
        memset(count_aex, 0, sizeof(count_aex));
        memset(monitor_aex, 0, sizeof(monitor_aex));
        aex_count=0;
        monitor_aex_count=0;
    }

    sgx_unregister_aex_handler(aex_handler);

    monitor_stopped = true;
    eprintf("Monitoring done.\r\n");
}

int ENode::add_sibling(std::string hostname, uint16_t _port)
{
    eprintf("Adding sibling %s:%d to node...\r\n", hostname, _port);
    if(hostname=="127.0.0.1"&&_port==port)
    {
        eprintf("Won't add self as a sibling.\r\n");
        return SUCCESS;
    }
    if(std::find(siblings.begin(), siblings.end(), std::make_pair(hostname, _port)) != siblings.end())
    {
        eprintf("Sibling already added.\r\n");
        return SUCCESS;
    }
    siblings.emplace_back(hostname, _port);
    eprintf("Sibling added.\r\n");

    const char* buff = "Sibling";
    size_t buff_len = strlen(buff);
    unsigned char buff_enc[buff_len + crypto_aead_aes256gcm_ABYTES];
    unsigned long long buff_len_enc = buff_len + crypto_aead_aes256gcm_ABYTES;
    int retval=encrypt((const unsigned char*)buff, buff_len, buff_enc, &buff_len_enc);
    eprintf("addSibling: Encrypting message to %s:%d: %s\r\n", hostname.c_str(), _port, buff);
    if(retval)
    {
        eprintf("addSibling: Encryption failed.\r\n");
    }
    else
    {
        eprintf("addSibling: Encrypted message of len %d: %s\r\n", buff_len_enc, buff_enc);
    }
    ssize_t sendStatus = sendto(sock, buff_enc, buff_len_enc, 0, hostname.c_str(), INET_ADDRSTRLEN, _port);
    if (sendStatus< 0) {
        eprintf("sending error...: %d\r\n", errno);
        sgx_thread_rwlock_wrlock(&socket_rwlock);
        close(sock);
        sgx_thread_rwlock_unlock(&socket_rwlock);
        return SENDING_ERROR;
    }
    return SUCCESS;
}

timespec ENode::get_timestamp()
{
    long long int mem_tsc=tsc;
    while((!calib_count || !calib_ts_ref || tainted || mem_tsc==tsc || tsc < tsc_ref) && !should_stop());
    timespec timestamp;
    long long total_nsec = (long long)((double)(tsc-tsc_ref)/tsc_freq);
    timestamp.tv_sec = (total_nsec+ts_ref.tv_nsec)/1000000000;
    timestamp.tv_sec += ts_ref.tv_sec;
    timestamp.tv_nsec = (total_nsec+ts_ref.tv_nsec)%1000000000; 
    return timestamp;
}