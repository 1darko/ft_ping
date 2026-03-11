
/*
    Obligatoire 
    You have to manage the -v -? options.
    
    Bonus 
    -s size
        Specify the number of data bytes to be sent. The default is 56, which translates into 64 ICMP data bytes when combined with the 8 bytes of ICMP header data.
    -D 
        Print timestamp (unix time + microseconds as in gettimeofday) before each line.

    -W deadline
        Specify a timeout, in seconds, before ping exits regardless of how many packets have been sent or received. In this case ping does not stop after count packet are sent, it waits either for deadline
        expire or until count probes are answered or for some error notification from network.

    -c count
        Stop after sending count ECHO_REQUEST packets. With deadline option, ping waits for count ECHO_REPLY packets, until the timeout expires.
    -f
        Flood ping. For every ECHO_REQUEST sent a period `.` is printed, while for
    -i time
        Wait interval seconds between sending each packet. The default is to wait for one second between each packet normally this option is only available to the super-user.
*/

/*
ping f6r6s4.clusters.42paris.fr
    Gestion adresse IP
*/
#include "ping.h"

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig)
{
    (void)sig;
    stop = 1;
};

void print_question_mark(){
    printf("Usage: sudo ft_ping [-v] [-?] [-D] [-w deadline] [-c count] <destination>\n");
    printf("  -v               Verbose output\n");
    printf("  -?               Show this help message\n");
    printf("  -D               Print timestamp before each line\n");
    printf("  -W timeout       Specify a time to wait for a response, in seconds.\n");
    printf("  -c count         Stop after sending count ECHO_REQUEST packets\n");
    printf("  -s size          Specify the number of data bytes to be sent. Needs to be between 0 and 1473 bytes.\n");
    printf("  -f               Enable flood ping (send packets as fast as possible)\n");
    printf("  -i time          Wait interval seconds between sending each packet\n");
    printf("  -q               Quiet output\n");
    printf("  -r               Record route\n");
    printf("Flags -f and -i are mutually exclusive.\n");
}

int init_sockets(options *opts, int *send_sock, int *sock_recv)
{
    *send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (*send_sock < 0)
        return (perror("send_sock"), 1);

    *sock_recv = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (*sock_recv < 0)
        return (perror("sock_recv"), 1);

    int on = 1;
    if (setsockopt(*send_sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
        return (perror("setsockopt IP_HDRINCL"), 1);

    if (opts->r)
        if (setsockopt(*send_sock, SOL_SOCKET, SO_DONTROUTE, &on, sizeof(on)) < 0)
            return (perror("setsockopt SO_DONTROUTE"), 1);

    struct timeval tv = { .tv_sec = opts->w ? atoi(opts->w) : 10, .tv_usec = 0 };
    if (setsockopt(*send_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
        return (perror("setsockopt SO_SNDTIMEO"), 1);
    if (setsockopt(*sock_recv, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        return (perror("setsockopt SO_RCVTIMEO"), 1);

    return 0;
}

int main(int ac, char **av)
{
    if (ac < 2) {
        fprintf(stderr, "Usage: sudo ft_ping [...] <destination>\n");
        return 1;
    }
    if (getuid() != 0) {
        fprintf(stderr, "ft_ping: must be run as root\n");
        return 1;
    }
    options    opts   = {0};
    const char *dst_ip;
    if (flag_checker(ac, av, &opts, &dst_ip))
        return 1;
    if (resolve_ip(dst_ip, &opts) != 0) {
        fprintf(stderr, "ft_ping: unknown host %s\n", dst_ip);
        return 1;
    }

    int send_sock, sock_recv;
    if (init_sockets(&opts, &send_sock, &sock_recv))
        return 1;

    struct sigaction sa = {0};
    sa.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        return (fprintf(stderr, "sigaction\n"), 1);

    uint16_t my_id = getpid() & 0xFFFF;
    char     packet[1500];
    struct iphdr   *iph;
    struct icmphdr *icmph;
    build_packet(packet, &iph, &icmph, &opts, my_id);

    struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family      = AF_INET;
    dest_addr.sin_addr.s_addr = iph->daddr;

    printf("PING %s (%s) %d data bytes", dst_ip, opts.ipv4, opts.s_value);
    if (opts.v)
        printf(", id 0x0%x = %d", htons(icmph->un.echo.id), my_id);
    printf("\n");

    ping_loop(send_sock, sock_recv, &opts, dst_ip, packet, icmph, &dest_addr, my_id);

    printf("--- Ping statistics for %s ---\n", dst_ip);
    printf("%d packets transmitted, %d packets received, %0.1f%% packet loss\n",
        opts.transmited_packages, opts.received_packages,
        ((opts.transmited_packages - opts.received_packages) /
         (float)opts.transmited_packages) * 100.0);

    close(send_sock);
    close(sock_recv);
    return 0;
}

// int main(int ac, char **av) {
//     if(ac < 2) {
//         fprintf(stderr, "Usage: sudo ft_ping [-v] [-?] [-D] [-s <size>] [-c <count>] [-q] [-f] [-i <interval>] [-r] <destination>\n");
//         return 1;
//     };
//     if(getuid() != 0){
//         fprintf(stderr, "ft_ping: must be run as root\n");
//         return 1;
//     };
//     options opts = {0};
//     const char *dst_ip;
//     if(flag_checker(ac, av, &opts, &dst_ip)) {
//         return 0;
//     };
//     if(resolve_ip(dst_ip, &opts) != 0){
//         fprintf(stderr, "ft_ping: unknown host %s\n", dst_ip);
//         return 1;
//     };
//     uint16_t my_id = getpid() & 0xFFFF;
//     uint16_t seq = 0;

//     // sending part -------------
//     int send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
//     if(send_sock < 0){
//         perror("send_sock");
//         return 1;
//     };
//     // receiving part ------------
//     int sock_recv = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
//     if(sock_recv < 0){
//         perror("sock_recv");
//         return 1;
//     };
    
//     int on = 1;
//     /*  send_sock : la socket qu'on modifie
//         IPPROTO_IP: le protocole qu'on modifie
//         IP_HDRINCL: option à modifier
//         &on: valeur à assigner */

//     if(setsockopt(send_sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
//         perror("setsockopt IP_HDRINCL");
//         return 1;
//     };
//     // Pour traceroute / record route
//     // if(setsockopt(send_sock, IPPROTO_IP, IP_OPTIONS, &on, sizeof(on)) < 0){
//     //     perror("setsockopt IP_OPTIONS");
//     //     return 1;
//     // };
//     if(opts.r){
//         if(setsockopt(send_sock, SOL_SOCKET, SO_DONTROUTE, &on, sizeof(on)) < 0){
//             perror("setsockopt SO_DONTROUTE");
//             return 1;
//         };
//     }
//     // TIMEOUT SETTINGS
//     struct timeval tv;
//     tv.tv_sec = opts.w ? atoi(opts.w) : 10;
//     tv.tv_usec = 0;
//     if(setsockopt(send_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv) < 0){
//         perror("setsockopt SO_SNDTIMEO");
//         return 1;
//     };  
//     // };
//     if(setsockopt(sock_recv, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv) < 0){
//         perror("setsockopt SO_RCVTIMEO");
//         return 1;
//     };
//     char packet_to_send[1500];
//     memset(packet_to_send, 0, sizeof(packet_to_send));

//     // Header IP et ICMP
//     struct iphdr *iph = (struct iphdr *)packet_to_send;
//     struct icmphdr *icmph = (struct icmphdr *)(packet_to_send + sizeof(struct iphdr));
//     // Payload
//     char payload[opts.s_value];
//     bzero(payload, sizeof(payload));
//     int payload_len = opts.s_value;
//     // ICMP Header
//     // for(uint16_t seq = 0; seq < opts.c; seq++)
//     // {
//     icmph->type = ICMP_ECHO;
//     icmph->code = 0;
//     icmph->un.echo.id = htons(my_id);
//     icmph->un.echo.sequence = htons(seq); // a mettre dans une boucle plus tard
//     // }
//     icmph->checksum = 0;
//     memcpy(packet_to_send + sizeof(struct iphdr) + sizeof(struct icmphdr), payload, payload_len);
//     icmph->checksum = checksum(icmph, sizeof(struct icmphdr) + payload_len);

//     // IP HEADER

//     iph->ihl = 5; // Internet Header Len
//     iph->version = 4; // IPv4
//     iph->tos = 0; // Service Type
//     iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + payload_len); // Total len
//     iph->id = htons(0); // ID of this packet, no need to be unique, we dont fragment
//     iph->frag_off = 0; // Offset fragementation
//     iph->ttl = 64; // Time to live , maybe add as bonus ? 
//     iph->protocol = IPPROTO_ICMP;
//     // iph->check = 0; 
//     iph->saddr = inet_addr("0.0.0.0"); // kernel will fill the correct source IP
//     iph->daddr = inet_addr(opts.ipv4); // destination IP
//     //    opts.r ? (iph->daddr = inet_addr(dst_ip)) : (iph->daddr = inet_addr(opts.ipv4)); // destination IP

//     // iph->check = checksum(iph, sizeof(struct iphdr));

//     // Send the packet

//     struct sockaddr_in dest_addr;
//     bzero(&dest_addr, sizeof(dest_addr));
//     dest_addr.sin_family = AF_INET; // IPv4
//     dest_addr.sin_addr.s_addr = iph->daddr; // Destination IP

//     // printf("Size payload: %lu bytes\n",
//     //        payload_len);
//     // printf("Size of icmphdr: %lu bytes\n",
//     //        sizeof(struct icmphdr));
//     // printf("Size of iphdr: %lu bytes\n",
//     //     sizeof(struct iphdr));

//     // Sending packet(s)
//     struct sigaction sa;
//     memset(&sa, 0, sizeof(sa));
//     sa.sa_handler = handle_sigint;
//     if (sigaction(SIGINT, &sa, NULL) == -1) {
//         fprintf(stderr, "sigaction");
//         return 1;
//     }

//     char buf[1500];
//     struct timeval tv_send, tv_recv;
//     printf("PING %s (%s) %d data bytes", dst_ip, opts.ipv4, payload_len);
//     if(opts.v)
//         printf(", id 0x0%x = %d", htons(icmph->un.echo.id), my_id);
//     printf("\n");
//     for(uint16_t seq = 0; seq < opts.c_value; seq++)
//     {   if(stop)
//             break;
//         icmph->un.echo.sequence = htons(seq);
//         icmph->checksum = 0;
//         icmph->checksum = checksum(icmph, sizeof(struct icmphdr) + payload_len);
//         gettimeofday(&tv_send, NULL);
//         if(sendto(send_sock, packet_to_send, sizeof(struct iphdr) + sizeof(struct icmphdr) +\
//                payload_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0){
//                 perror("sendto");
//                 return 1;
//                }
//         opts.transmited_packages++;
//         // printf("Sent ICMP Echo Request seq=%d to %s\n", seq, dst_ip);
//         while(1 && !stop){
//             ssize_t bytes_received = recv(sock_recv, buf, sizeof(buf), 0);
//             if(bytes_received < 0){
//                 if(errno == 11){
//                     if(opts.v) printf("From %s (%s) icmp_seq=%d Time to live exceeded\n", dst_ip, opts.ipv4, seq);
//                     break;
//                 }
//                 perror("recv");
//                 continue;   
//             };
//             struct iphdr *recv_iph = (struct iphdr *)buf;
//             ssize_t ip_header_len = recv_iph->ihl * 4;
//             struct icmphdr *recv_icmph = (struct icmphdr *)(buf + ip_header_len);
//             if(recv_icmph->type == ICMP_ECHOREPLY && recv_icmph->un.echo.id == htons(my_id)\
//             && recv_icmph->un.echo.sequence == htons(seq)){
//                 //64 bytes from 8.8.8.8: icmp_seq=0 ttl=63 time=1.778 ms
//                 if(!opts.q){
//                     gettimeofday(&tv_recv, NULL);
//                     if(opts.D){
//                         printf("[%ld.%06ld] ", tv_recv.tv_sec, tv_recv.tv_usec);
//                     }
//                     printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
//                         bytes_received - ip_header_len,
//                         dst_ip,
//                         seq,
//                         recv_iph->ttl,
//                         (tv_recv.tv_sec - tv_send.tv_sec) * 1000.0 + (tv_recv.tv_usec - tv_send.tv_usec) / 1000.0);
//                         if(!opts.flood && seq + 1 < opts.c_value)
//                             sleep(opts.i_is_set ? atoi(opts.i) : 1);
//                 }
//                 opts.received_packages++;
//                 break;
//             };

//         }
//     };
//     printf("--- Ping statistics for %s ---\n", dst_ip);
//     printf("%d packets transmitted, %d packets received, %0.1f%% packet loss\n",
//         opts.transmited_packages,
//         opts.received_packages,
//         ((opts.transmited_packages - opts.received_packages) / (float)opts.transmited_packages) * 100.0);
//     close(send_sock);
//     close(sock_recv);
//     return 0;
// }

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
};


int resolve_ip(const char *name_or_ip, options *opts)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    int rc;
    // char ipstr[INET_ADDRSTRLEN];
    // if (!name_or_ip || !(*resolved_ip))
        // return 1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        /* force IPv4 */
    hints.ai_socktype = 0;            /* pas de contrainte */

    rc = getaddrinfo(name_or_ip, NULL, &hints, &res);
    if(rc == 0)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;
        // *resolved_ip = sin->sin_addr.s_addr; /* déjà en network byte order */
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin->sin_addr, ipstr, sizeof(ipstr));
        // printf("%s -> %s\n", name_or_ip, ipstr);
        strcpy(opts->ipv4, ipstr);
        freeaddrinfo(res);
        return 0;
    }
    return 1;
}