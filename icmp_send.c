#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include "icmp_send.h"
#include "utils.h"

void send_icmp(const char *dest, int ttl) {
    struct sockaddr_in recipient; //adres gniazda odbierającego
    memset(&recipient, 0, sizeof(recipient));
    recipient.sin_family = AF_INET; //IPv4
    int ip_conversion = inet_pton(AF_INET, dest, &recipient.sin_addr); //konwersja stringa IP na postać binarną
    if (ip_conversion != 1) {
        ERROR("Błąd adresu IP!");
    }
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); //gniazdo wysyłające
    if (sockfd < 0) {
        ERROR("Błąd gniazda!");
    }
    int ttl_change = setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)); //konwersja TTL jak na wykładzie
    if (ttl_change < 0) {
       ERROR("Błąd ustawienia TTL!");
    }
    for (int i = 0; i < 3; i++) {
        struct icmp header; //konstruujemy nagłówek do wysłania komunikatu ICMP
        header.icmp_type = ICMP_ECHO;
        header.icmp_code = 0;
        header.icmp_hun.ih_idseq.icd_id = getpid() & 0xFFFF; //jak na wykładzie, 16 najmniej znaczących bitów PID
        header.icmp_hun.ih_idseq.icd_seq = ttl * 100 + i; //żeby się odróżniało
        header.icmp_cksum = 0; 
        header.icmp_cksum = compute_icmp_checksum((u_int16_t*)&header, sizeof(header));
        ssize_t bytes_sent = sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr*)&recipient, sizeof(recipient));
        if (bytes_sent < 0) {
            ERROR("Błąd wysłania pakietu!");
        }
    }
    close(sockfd);
}

// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         printf("Musisz podać adres IP!\n");
//         return -1;
//     }
//     for (int i = 1; i <= 30; i++) {
//         send_icmp(argv[1], i);
//         printf("Pomyślnie wysłano: %d\n", i);
//     }
//     return 0;
// }



