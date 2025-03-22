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
#include "icmp_receive.h"
#include "utils.h"

void receive_icmp(const char *addr, int ttl) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); //nowy socket jako deskryptor pliku
    //AF_INET - IPv4, SOCK_RAW - surowe gniazdo, IPPROTO_ICMP - definiujemy protokół jako ICMP
    if (sockfd < 0) {
        ERROR("Błąd gniazda przy odbieraniu!");
    }
    struct pollfd ps; //uzupełniamy pola pollfd jak na wykładzie
    ps.fd = sockfd; 
    ps.events = POLLIN;
    ps.revents = 0;
    int received = 0;
    long long rtt_times[3];
    char sender_ip_str[3][20];
    long long start = get_current_time_ms();
    long long deadline = start + 1000;
    for (int i = 0; i < 3; i++) {
        long long time_left = deadline - get_current_time_ms();
        int ready = poll(&ps, 1, time_left);
        if (ready < 0) {
            ERROR("Błąd poll!");
        } else if (ready == 0) {
            printf("%d. *\n", ttl);
            close(sockfd);
            return;
        }
        struct sockaddr_in sender; //struct opisujący adres wysyłającego
        socklen_t sender_len = sizeof(sender); //długość adresu
        u_int8_t buffer[IP_MAXPACKET]; //bufor, w którym znajdzie się adres
        ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
        //odbieranie pakietu z danego gniazda do danego bufora 
        if (packet_len < 0) {
            if (errno == EWOULDBLOCK) {
                continue;
            }
            ERROR("Błąd recvfrom!");
        }
        inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str[received], sizeof(sender_ip_str[received])); 
        //zamiana adresu IP z postaci bajtowej na postać CIDR
        struct ip *ip_header = (struct ip*) buffer;
        ssize_t ip_header_len = 4 * (ssize_t)(ip_header->ip_hl);
        struct icmp *icmp_header = (struct icmp*)(buffer + ip_header_len);
        int valid = 0;
        if (icmp_header->icmp_type == ICMP_ECHOREPLY && icmp_header->icmp_hun.ih_idseq.icd_id == (getpid() & 0xFFFF) && icmp_header->icmp_hun.ih_idseq.icd_seq / 100 == ttl) {
            valid = 1;
        } else if(icmp_header->icmp_type == ICMP_TIME_EXCEEDED) {
            struct ip *inner = (struct ip *)&icmp_header->icmp_data;
            u_int8_t *inner_header = icmp_header->icmp_data + 4 * inner->ip_hl;
            struct icmp *inner_icmp = (struct icmp *)inner_header;
            if (inner_icmp->icmp_hun.ih_idseq.icd_id == (getpid() & 0xFFFF) && inner_icmp->icmp_hun.ih_idseq.icd_seq / 100 == ttl) {
                valid = 1;
            }
        }
        if (valid == 1) {
            long long time_r = get_current_time_ms() - start;
            rtt_times[received] = time_r;
            received++;
        }
    }
    if (received == 0) {
        printf("%d. *\n", ttl);
        return;
    }
    printf("%d. ", ttl);
    for (int i = 0; i < received; i++) {
        printf("%s ", sender_ip_str[i]);
    }
    if (received < 3) {
        printf("???\n");
    } else {
        printf("%lldms ", (rtt_times[0] + rtt_times[1] + rtt_times[2]) / received);
        printf("\n");
    }
    if (received > 0 && strcmp(sender_ip_str[0], addr) == 0) {
        exit(0);
    }
}