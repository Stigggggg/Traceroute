#include <stdio.h>
#include <stdlib.h>
#include "icmp_send.h"
#include "icmp_receive.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Format: %s IP\n", argv[0]);
        
        exit(1);
    }
    for (int i = 1; i <= 30; i++) { //i == TTL
        send_icmp(argv[1], i);
        receive_icmp(argv[1], i);
    }
    return 0;
}