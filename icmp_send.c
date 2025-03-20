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
#include <sys/time.h>

u_int16_t compute_icmp_checksum(const void *buff, int length);

void ERROR(const char* str); //na to zmienic errory

void send_icmp(const char *dest, int ttl) {
    struct sockaddr_in recipient; //adres gniazda odbierającego
    memset(&recipient, 0, sizeof(recipient)); //alokacja pamięci
    recipient.sin_family = AF_INET; //IPv4
    int ip_conversion = inet_pton(AF_INET, dest, &recipient.sin_addr); //konwersja stringa IP na postać binarną
    if (ip_conversion != 1) {
        printf("Błąd adresu IP!");
        exit(1);
    }
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); //gniazdo wysyłające
    if (sockfd < 0) {
        printf("Błąd gniazda!");
        exit(1);
    }
    int ttl_change = setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)); //konwersja TTL jak na wykładzie
    if (ttl_change < 0) {
        printf("Błąd ustawienia TTL!");
        exit(1);
    }
    for (int i = 0; i < 3; i++) {
        struct icmp header; //konstruujemy nagłówek do wysłania komunikatu ICMP
        header.icmp_type = ICMP_ECHO;
        header.icmp_code = 0;
        header.icmp_hun.ih_idseq.icd_id = getpid() & 0xFFFF; //jak na wykładzie, 16 najmniej znaczących bitów PID
        header.icmp_hun.ih_idseq.icd_seq = ttl * 100 + i; //żeby się odróżniało
        header.icmp_cksum = 0;
        struct timeval current_time; //obliczanie czasu wysłania
        gettimeofday(&current_time, NULL);
        // printf("seconds: %ld\nmicroseconds: %ld\n", current_time.tv_sec, current_time.tv_usec);      
        header.icmp_cksum = compute_icmp_checksum((u_int16_t*)&header, sizeof(header));
        ssize_t bytes_sent = sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr*)&recipient, sizeof(recipient));
        if (bytes_sent < 0) {
            printf("Błąd wysyłania pakietu!");
            close(sockfd);
            exit(1);
        }
    }
    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Musisz podać adres IP!\n");
        return -1;
    }
    for (int i = 1; i <= 30; i++) {
        send_icmp(argv[1], i);
        printf("Pomyślnie wysłano: %d\n", i);
    }
    return 0;
}



