#include "ping.h"

void build_packet(char *packet, struct iphdr **iph, struct icmphdr **icmph,
                  options *opts, uint16_t my_id)
{
    memset(packet, 0, 1500);
    *iph   = (struct iphdr *)packet;
    *icmph = (struct icmphdr *)(packet + sizeof(struct iphdr));

    // ICMP
    (*icmph)->type          = ICMP_ECHO;
    (*icmph)->code          = 0;
    (*icmph)->un.echo.id    = htons(my_id);
    (*icmph)->un.echo.sequence = 0;

    // Payload
    char payload[opts->s_value];
    bzero(payload, opts->s_value);
    memcpy(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), payload, opts->s_value);

    // IP
    (*iph)->ihl      = 5;
    (*iph)->version  = 4;
    (*iph)->tos      = 0;
    (*iph)->tot_len  = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + opts->s_value);
    (*iph)->id       = 0;
    (*iph)->frag_off = 0;
    (*iph)->ttl      = 64;
    (*iph)->protocol = IPPROTO_ICMP;
    (*iph)->saddr    = inet_addr("0.0.0.0");
    (*iph)->daddr    = inet_addr(opts->ipv4);
}

int receive_reply(int sock_recv, options *opts, const char *dst_ip,
                  uint16_t my_id, uint16_t seq, struct timeval tv_send)
{
    char buf[1500];

    while (!stop) {
        ssize_t n = recv(sock_recv, buf, sizeof(buf), 0);
        if (n < 0) {
            if (errno == EAGAIN) {
                // if (opts->v)
                //     printf("From %s (%s) icmp_seq=%d Time to live exceeded\n",
                //            dst_ip, opts->ipv4, seq);
                return 0;
            }
            perror("recv");
            continue;
        }
        struct iphdr   *recv_iph   = (struct iphdr *)buf;
        ssize_t         ip_hlen    = recv_iph->ihl * 4;
        struct icmphdr *recv_icmph = (struct icmphdr *)(buf + ip_hlen);
        // char src_ip[INET_ADDRSTRLEN];
        // inet_ntop(AF_INET, &recv_iph->saddr, src_ip, sizeof(src_ip));
        // printf("DEBUG recv: src=%s type=%d id=%d seq=%d ttl=%d\n",
        //     src_ip,
        //     recv_icmph->type,
        //     ntohs(recv_icmph->un.echo.id),
        //     ntohs(recv_icmph->un.echo.sequence),
        //     recv_iph->ttl);
        if (recv_icmph->type           == ICMP_ECHOREPLY
         && recv_icmph->un.echo.id     == htons(my_id)
         && recv_icmph->un.echo.sequence == htons(seq))
        {
            opts->received_packages++;
            if (!opts->q) {
                struct timeval tv_recv;
                gettimeofday(&tv_recv, NULL);
                if (opts->D)
                    printf("[%ld.%06ld] ", tv_recv.tv_sec, tv_recv.tv_usec);
                printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
                    n - ip_hlen, dst_ip, seq + 1, recv_iph->ttl,
                    (tv_recv.tv_sec  - tv_send.tv_sec)  * 1000.0 +
                    (tv_recv.tv_usec - tv_send.tv_usec) / 1000.0);
            }
            return 1; // got response
        }
    }
    return 0;
}

int ping_loop(int send_sock, int sock_recv, options *opts,
              const char *dst_ip, char *packet,
              struct icmphdr *icmph, struct sockaddr_in *dest_addr,
              uint16_t my_id)
{
    struct timeval tv_send;

    for (uint16_t seq = 0; seq < opts->c_value && !stop; seq++) {
        icmph->un.echo.sequence = htons(seq);
        icmph->checksum = 0;
        icmph->checksum = checksum(icmph, sizeof(struct icmphdr) + opts->s_value);

        gettimeofday(&tv_send, NULL);
        if (sendto(send_sock, packet,
                   sizeof(struct iphdr) + sizeof(struct icmphdr) + opts->s_value,
                   0, (struct sockaddr *)dest_addr, sizeof(*dest_addr)) < 0)
            return (perror("sendto"), 1);
        opts->transmited_packages++;

        int got_reply = receive_reply(sock_recv, opts, dst_ip, my_id, seq, tv_send);
        if (got_reply && !opts->flood && seq + 1 < opts->c_value)
            sleep(opts->i_is_set ? atoi(opts->i) : 1);
    }
    return 0;
}