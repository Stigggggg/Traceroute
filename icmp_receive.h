#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <poll.h>

void ERROR(const char* str);

void print_as_bytes (unsigned char* buff, ssize_t length);

void receive_icmp();