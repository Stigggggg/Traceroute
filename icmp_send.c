#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>

u_int16_t compute_icmp_checksum(const void *buff, int length);

void send_icmp(const char *dest, int ttl) {
    struct sockaddr_in recipient;
    memset(&recipient, 0, sizeof(recipient));
    recipient.sin_family = AF_INET;
    int ip_conversion = inet_pton(AF_INET, dest, &recipient.sin_addr);
    if (ip_conversion != 1) {
        printf("Błąd adresu IP!");
        exit(1);
    }
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        printf("Błąd gniazda!");
        exit(1);
    }
    int ttl_change = setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
    if (ttl_change < 0) {
        printf("Błąd ustawienia TTL!");
        exit(1);
    }
    for (int i = 0; i < 3; i++) {
        struct icmp header;
        header.icmp_type = ICMP_ECHO;
        header.icmp_code = 0;
        header.icmp_hun.ih_idseq.icd_id = getpid() & 0xFFFF;
        header.icmp_hun.ih_idseq.icd_seq = i;
        header.icmp_cksum = 0;
        header.icmp_cksum = compute_icmp_checksum((u_int16_t*)&header, sizeof(header));
        ssize_t bytes_sent = sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr*)&recipient, sizeof(recipient));
        if (bytes_sent < 0) {
            printf("Błąd wysyłania pakietu!");
            exit(1);
        }
        printf("Pomyślnie wysłano!\n");
    }
    exit(0);
}

// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         fprintf(stderr, "Użycie: %s <adres_IP>\n", argv[0]);
//         return -1;
//     }
//     int ttl = 42;
//     send_icmp(argv[1], ttl);
//     return 0;
// }



