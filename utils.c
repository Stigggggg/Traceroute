#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "utils.h"

//funkcje z wykładu i pomocnicza będąca wrapperem czasu
u_int16_t compute_icmp_checksum(const void *buff, int length)
{
    const u_int16_t* ptr = buff;
    u_int32_t sum = 0;
    assert (length % 2 == 0);
    for (; length > 0; length -= 2)
        sum += *ptr++;
    sum = (sum >> 16U) + (sum & 0xffffU);
    return ~(sum + (sum >> 16U));
}

void ERROR(const char* str) {
    fprintf(stderr, "%s: %s\n", str, strerror(errno));  // NOLINT(*-err33-c)
    exit(EXIT_FAILURE);
}

void print_as_bytes (unsigned char* buff, ssize_t length) {
    for (ssize_t i = 0; i < length; i++, buff++)
        printf("%.2x ", *buff);
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}