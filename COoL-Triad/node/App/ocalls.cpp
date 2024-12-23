#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void ocall_print_string(const char *str)
{
    printf("%s", str);
}


void ocall_readTSC(long long* ts) {
    /*
    Read the TSC register
    */
    #if defined(_MSC_VER) // MSVC specific
        *ts = __rdtsc();
    #elif defined(__GNUC__) || defined(__clang__) // GCC or Clang specific
        unsigned int lo, hi;
        __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
        *ts = ((uint64_t)hi << 32) | lo;
    #else
    #error "Compiler not supported"
    #endif
}

void ocall_sleep(int sec) {
    sleep(sec);
}

void ocall_usleep(int usec) {
    usleep(usec);
}

void ocall_timespec_get(struct timespec* ts) {
    timespec_get(ts, TIME_UTC);
    char buff[100];
    strftime(buff, sizeof buff, "%D %T", gmtime(&(ts->tv_sec)));
    //printf("[utrst]> Current time: %s.%09ld UTC\n", buff, ts->tv_nsec);
}

void ocall_timespec_print(struct timespec* ts) {
    char buff[100];
    strftime(buff, sizeof buff, "%D %T", gmtime(&(ts->tv_sec)));
    printf("[utrst]> Time: %s.%09ld UTC\n", buff, ts->tv_nsec);
}

#ifdef __cplusplus
}
#endif