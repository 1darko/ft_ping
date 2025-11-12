#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <errno.h>
#define PAYLOAD_SIZE 56  // Taille du payload ICMP
// Calcule un checksum 16 bits
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <destination IP>\n", argv[0]);
        return 1;
    }

    const char *dst_ip = argv[1];
    uint16_t my_id = getpid() & 0xFFFF;
    uint16_t seq = 0;

    // --- Socket pour envoi ---
    int send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (send_sock < 0) {
        perror("socket send_sock");
        return 1;
    }

    int on = 1;
    if (setsockopt(send_sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("setsockopt IP_HDRINCL");
        return 1;
    }

    // --- Socket pour réception ---
    int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (recv_sock < 0) {
        perror("socket recv_sock");
        return 1;
    }

    // --- Construction du paquet ---
    char packet[1500];
    memset(packet, 0, sizeof(packet));
    

    struct iphdr *iph = (struct iphdr *)packet;
    struct icmphdr *icmph = (struct icmphdr *)(packet + sizeof(struct iphdr));

    char payload[PAYLOAD_SIZE];
    bzero(payload, sizeof(payload));
    // memset(payload + strlen(payload), 0, sizeof(payload) - strlen(payload));
    int payload_len = PAYLOAD_SIZE;

    // ICMP Header
    icmph->type = ICMP_ECHO;
    icmph->code = 0;
    icmph->un.echo.id = htons(my_id);
    icmph->un.echo.sequence = htons(seq);
    icmph->checksum = 0;
    memcpy(packet + sizeof(struct iphdr) + sizeof(struct icmphdr),
           payload, payload_len);
    icmph->checksum = checksum(icmph, sizeof(struct icmphdr) + payload_len);

    // IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + payload_len);
    iph->id = htons(0);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_ICMP;
    iph->check = 0;
    iph->saddr = inet_addr("0.0.0.0");  // le kernel choisira la bonne IP source
    iph->daddr = inet_addr(dst_ip);
    iph->check = checksum(iph, sizeof(struct iphdr));

    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr(dst_ip);

    // printf("Size of packet to send: %lu bytes\n",
    //        sizeof(struct iphdr) + sizeof(struct icmphdr) + payload_len);
    printf("Size payload: %lu bytes\n",
           payload_len);
    printf("Size of icmphdr: %lu bytes\n",
           sizeof(struct icmphdr));
    printf("Size of iphdr: %lu bytes\n",
        sizeof(struct iphdr));



    // --- Envoi ---
    if (sendto(send_sock, packet,
               sizeof(struct iphdr) + sizeof(struct icmphdr) + payload_len,
               0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("sendto");
        return 1;
    }

    printf("ICMP Echo Request sent to %s (id=%d seq=%d)\n",
           dst_ip, my_id, seq);

    // --- Réception ---
    char buf[1500];
    while (1) {
        ssize_t len = recv(recv_sock, buf, sizeof(buf), 0);
        if (len < 0) {
            perror("recv");
            continue;
        }

        struct iphdr *r_iph = (struct iphdr *)buf;
        int iphdr_len = r_iph->ihl * 4;
        struct icmphdr *r_icmp = (struct icmphdr *)(buf + iphdr_len);

        if (r_icmp->type == ICMP_ECHOREPLY &&
            ntohs(r_icmp->un.echo.id) == my_id &&
            ntohs(r_icmp->un.echo.sequence) == seq) {
            printf("Got ICMP Echo Reply from %s: id=%d seq=%d\n",
                   inet_ntoa(*(struct in_addr *)&r_iph->saddr),
                   my_id, seq);
            break;
        }
    }

    close(send_sock);
    close(recv_sock);
    return 0;
}
