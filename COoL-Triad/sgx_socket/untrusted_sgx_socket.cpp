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


#include <unistd.h>
#include "sys/socket.h"
#include "netinet/in.h"
#include <cstdio>
#include <arpa/inet.h>
#include <cstring>
/* ocalls to use socket APIs , call socket syscalls */

#ifdef __cplusplus
extern "C" {
#endif

int u_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

int u_connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen)
{
    return connect(sockfd, servaddr, addrlen);
}

int u_bind(int fd, const struct sockaddr *addr, socklen_t len)
{
    return bind(fd, addr, len);
}

int u_listen(int fd, int n)
{
    return listen(fd, n);
}

int u_accept(
			int fd, 
			struct sockaddr *addr, 
			socklen_t addrlen_in,
			socklen_t *addrlen_out
			)
{
    int ret = -1;

    if ((ret = accept(fd, addr, &addrlen_in)) != -1) 
    {
        if (addrlen_out) 
            *addrlen_out = addrlen_in;
    }
    return ret;
}

ssize_t u_send(int sockfd, const void *buf, size_t nbytes, int flags)
{
    return send(sockfd, buf, nbytes, flags);
}

ssize_t u_recv(int sockfd, void *buf, size_t nbytes, int flags)
{
    return recv(sockfd, buf, nbytes, flags);
}

int u_setsockopt(
			int sockfd, 
			int level, 
			int optname, 
			const void *optval,
			socklen_t optlen
			)
{
    return setsockopt(sockfd, level, optname, optval, optlen);
}

int u_close(int fd)
{
    return close(fd);
}

ssize_t u_sendto(int sockfd, const void *buf, size_t len, int flags, const char* ip, int iplen, int port)
{
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(ip);
    socklen_t addrlen = sizeof(dest_addr);
    return sendto(sockfd, buf, len, flags, (struct sockaddr*)&dest_addr, addrlen);
}

ssize_t u_recvfrom(int sockfd, void *buf, size_t len, int flags, char* ip, int iplen, int* port)
{
    struct sockaddr_in cliAddr;
    socklen_t cliAddrLen = sizeof(cliAddr);
    char buff[1024] = {0};
    //printf("[utrst]> %d, %p, %ld, %d, %p, %p\r\n", sockfd, buf, len, flags, (struct sockaddr*)&cliAddr, &cliAddrLen);
    ssize_t ret=recvfrom(sockfd, buf, len, flags, (struct sockaddr*)&cliAddr, &cliAddrLen);
    char *ipaddr = inet_ntoa(cliAddr.sin_addr);
    //printf("[utrst]> %d, %p, %ld, %d, %s, %d\r\n", sockfd, buf, len, flags, ipaddr, ntohs(cliAddr.sin_port));
    strcpy(ip, ipaddr);
    *port = ntohs(cliAddr.sin_port);
    //printf("[utrst]> %d, %p, %ld, %d, %s, %d\r\n", sockfd, buf, len, flags, ip, *port);
    //printf("[utrst]> msg size: %ld\r\n", ret);
    return ret;
}

#ifdef __cplusplus
}
#endif