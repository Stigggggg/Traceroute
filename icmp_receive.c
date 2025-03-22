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
    //nowy socket jako deskryptor pliku
    //AF_INET - IPv4, SOCK_RAW - surowe gniazdo, IPPROTO_ICMP - definiujemy protokół jako ICMP
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        ERROR("Błąd gniazda przy odbieraniu!");
    }
    //uzupełniamy pola pollfd jak na wykładzie
    struct pollfd ps;
    ps.fd = sockfd; 
    ps.events = POLLIN;
    ps.revents = 0;
    int received = 0;
    long long rtt_times[3] = {0};
    int unique_ips = 0;
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
        //struct opisujący adres wysyłającego
        struct sockaddr_in sender; 
        socklen_t sender_len = sizeof(sender);
        u_int8_t buffer[IP_MAXPACKET];
        //odbieranie pakietu z danego gniazda do danego bufora 
        ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
        if (packet_len < 0) {
            if (errno == EWOULDBLOCK) {
                continue;
            }
            ERROR("Błąd recvfrom!");
        }
        char sender_ip[20];
        //zamiana adresu IP z postaci bajtowej na postać CIDR
        inet_ntop(AF_INET, &(sender.sin_addr), sender_ip, sizeof(sender_ip));
        //zapobieganie powtarzaniu się IP przy wypisywaniu 
        int is_duplicate = 0;
        for (int j = 0; j < unique_ips; j++) {
            if (strcmp(sender_ip_str[j], sender_ip) == 0) {
                is_duplicate = 1;
                break;
            }
        }
        if (is_duplicate == 1) {
            continue;
        }
        strcpy(sender_ip_str[unique_ips], sender_ip);
        unique_ips++;
        //pobieranie nagłówka IP z bufora
        struct ip *ip_header = (struct ip*) buffer;
        ssize_t ip_header_len = 4 * (ssize_t)(ip_header->ip_hl);
        //pobieranie nagłówka ICMP
        struct icmp *icmp_header = (struct icmp*)(buffer + ip_header_len);
        int valid = 0;
        //obsługa ECHOREPLY
        if (icmp_header->icmp_type == ICMP_ECHOREPLY && icmp_header->icmp_hun.ih_idseq.icd_id == (getpid() & 0xFFFF) && icmp_header->icmp_hun.ih_idseq.icd_seq / 100 == ttl) {
            valid = 1;
        } else if(icmp_header->icmp_type == ICMP_TIME_EXCEEDED) { //obsługa TIME EXCEEDED
            struct ip *inner = (struct ip *)&icmp_header->icmp_data;
            u_int8_t *inner_header = icmp_header->icmp_data + 4 * inner->ip_hl;
            struct icmp *inner_icmp = (struct icmp *)inner_header;
            if (inner_icmp->icmp_hun.ih_idseq.icd_id == (getpid() & 0xFFFF) && inner_icmp->icmp_hun.ih_idseq.icd_seq / 100 == ttl) {
                valid = 1;
            }
        }
        //jeśli pakiet jest ok, to zliczamy go, liczymy mu rtt
        if (valid == 1) {
            long long time_r = get_current_time_ms() - start;
            rtt_times[received] = time_r;
            received++;
        }
    }
    //jeśli 0 odebranych to wypisujemy *
    if (received == 0) {
        printf("%d. *\n", ttl);
        return;
    }
    printf("%d. ", ttl);
    //IP bez powtórzeń
    for (int i = 0; i < unique_ips; i++) {
        printf("%s ", sender_ip_str[i]);
    }
    //sumaryczne RTT do średniej
    long long sum = 0;
    for (int i = 0; i < received; i++) {
        sum += rtt_times[i];
    }
    if (received > 0) {
        printf("%lldms\n", sum / received);
    } else {
        printf("???\n");
    }
    //jeśli doszliśmy do celu, to koniec
    if (received > 0 && strcmp(sender_ip_str[0], addr) == 0) {
        exit(0);
    }
}