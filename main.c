
/*
    Obligatoire 
    You have to manage the -v -? options.
    
    Bonus 
    -s size
        Specify the number of data bytes to be sent. The default is 56, which translates into 64 ICMP data bytes when combined with the 8 bytes of ICMP header data.
    -D 
        Print timestamp (unix time + microseconds as in gettimeofday) before each line.

    -w deadline
        Specify a timeout, in seconds, before ping exits regardless of how many packets have been sent or received. In this case ping does not stop after count packet are sent, it waits either for deadline
        expire or until count probes are answered or for some error notification from network.

    -c count
        Stop after sending count ECHO_REQUEST packets. With deadline option, ping waits for count ECHO_REPLY packets, until the timeout expires.
    -f
        Flood ping. For every ECHO_REQUEST sent a period `.` is printed, while for
    -i time
        Wait interval seconds between sending each packet. The default is to wait for one second between each packet normally this option is only available to the super-user.
*/

#include "ping.h"
unsigned short checksum(void *b, int len);
void print_question_mark(){
    printf("Usage: sudo ft_ping [-v] [-?] [-D] [-w deadline] [-c count] <destination>\n");
    printf("  -v               Verbose output\n");
    printf("  -?               Show this help message\n");
    printf("  -D               Print timestamp before each line\n");
    // printf("  -w deadline      Specify a timeout, in seconds, before ping exits\n");
    printf("  -c count         Stop after sending count ECHO_REQUEST packets\n");
    printf("  -s size          Specify the number of data bytes to be sent\n");
    printf("  -f               Enable flood ping (send packets as fast as possible)\n");
    printf("  -i time         Wait interval seconds between sending each packet\n");
    printf("Flags -f and -i are mutually exclusive.\n");
}

int isnumeric(const char *str){
    while(*str){
        if(*str < '0' || *str > '9')
            return 0; 
        str++;
    }
    return 1;
}

void value_error(const char *value, char near_char, int type){
    fprintf(stderr, "ping: invalid value (`%s' near `%c')\n", type ? value + 2 : value, near_char);
    fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
}

int int_overflow(char *a)
{
    if(!a)
        return 0;
    if(strlen(a) > 10)
        return 1;
    if(strlen(a) == 10){
        if(strcmp(a, "2147483647") > 0)
            return 1;
    }
    return 0;
}
int flag_checker2(options *opts, char **av)
{
    if(opts->s < 0 || opts->s > 1473){
        fprintf(stderr, "ft_ping: invalid payload size '%d'\n", opts->s);
        return 1;
    }
    if(opts->flood && opts->i_is_set){
        fprintf(stderr, "ft_ping: options '-f' and '-i' are mutually exclusive\n");
        return 1;
    }
    printf("C: -%s-\n", opts->c);
    if(int_overflow(opts->c)){
        fprintf(stderr, "ft_ping: invalid count '%d'\n", opts->c);
        return 1;
    }
    printf("bug\n");
    if(int_overflow(opts->i)){
        fprintf(stderr, "ft_ping: invalid interval '%s'\n", opts->i);
        return 1;
    }
    return 0;

}
int flag_checker(int ac, char **av, options *opts, const char **dst_ip)
{
    opts->s = PAYLOAD_SIZE; // to add option -s for payload size later
    // if bigger than 1473 or lesser than 0, should error out
    // -c to add
    // -w to add
    int break_flag;
    int ip_found = 0;
    for(int i = 1; i < ac; i++){
        break_flag = 0;
        if(av[i][0] == '-'){
            for(int j = 1; av[i][j] != '\0' && !break_flag; j++){
                switch(av[i][j]){
                    case 'v':
                        opts->v = 1;
                        break;  
                    case '?':
                        print_question_mark();
                        return 1;
                    case 'D':
                        opts->D = 1;
                        break;
                    case 'f':
                        opts->flood = 1;
                        break;
                    case 'i':
                        if(av[i][j + 1] != '\0'){
                            if(!isnumeric(&av[i][j + 1])){
                                return (value_error(av[i], 'i', 1), 1);
                            }
                            opts->i = &av[i][j + 1];
                            opts->i_is_set = 1;
                            break_flag = 1;
                            break;
                        }
                        else if(++i < ac && av[i][0] != '\0') {
                            if(!isnumeric(av[i])){
                                return (value_error(av[i], 'i', 0), 1);
                            }
                            opts->i = av[i];
                            opts->i_is_set = 1;
                        }
                        else{
                            fprintf(stderr, "ft_ping: option '-i' requires an argument\n");
                            return 1;
                        }
                        break_flag = 1;
                        break;
                    case 'c':
                        if(av[i][j + 1] != '\0'){
                            if(!isnumeric(&av[i][j + 1])){
                                return (value_error(av[i], 'c', 1), 1);
                            }
                            opts->c = &av[i][j + 1];
                        }
                        else if(++i < ac && av[i][0] != '\0') {
                            if(!isnumeric(av[i])){
                                return (value_error(av[i], 'c', 0), 1);
                            }
                            opts->c = av[i];
                        }
                        else{
                            fprintf(stderr, "ft_ping: option '-c' requires an argument\n");
                            return 1;
                        }
                        break_flag = 1;
                        break;
                    case 's':
                        if(av[i][j + 1] != '\0'){
                            if(!isnumeric(&av[i][j + 1])){
                                return (value_error(av[i], 's', 1), 1);
                            }
                            opts->s = &av[i][j + 1];
                        }
                        else if(++i < ac && av[i][0] != '\0') {
                            if(!isnumeric(av[i])){
                                return (value_error(av[i], 's', 0), 1);
                            }
                            opts->s = av[i];
                        }
                        else{
                            fprintf(stderr, "ft_ping: option '-s' requires an argument\n");
                            return 1;
                        }
                        break_flag = 1;
                        break;
                    default:
                        fprintf(stderr, "ft_ping: invalid option -- '%c'\n", av[i][j]);
                        fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
                        return 1;
                    }
                }
            }
            else if(ip_found == 0){
                *dst_ip = av[i];
                ip_found = 1;
            }
            else{
                fprintf(stderr, "ft_ping: extra argument '%s'\n", av[i]);
                fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
                return 1;
            }
        }
        if(!ip_found){
            fprintf(stderr, "ft_ping: destination address required\n");
            fprintf(stderr, "Try 'ft_ping -?' for more information.\n");
            return 1;
    }
    if(flag_checker2(opts, av))
        return 1;
    opts->s_value = (opts->s ? atoi(opts->s) : PAYLOAD_SIZE);
    opts->c_value = (opts->c ? atoi(opts->c) : INT_MAX);
    return 0;
};

int main(int ac, char **av) {
    if(ac < 2) {
        fprintf(stderr, "Usage: sudo ft_ping [-v] [-?] [-D] [-w deadline] [-c count] <destination>\n");
        return 1;
    }
    options opts = {0};
    const char *dst_ip;
    if(flag_checker(ac, av, &opts, &dst_ip)) {
        return 0;
    }
    uint16_t my_id = getpid() & 0xFFFF;
    uint16_t seq = 0;

    // sending part -------------
    int send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(send_sock < 0){
        perror("send_sock");
        return 1;
    };

    int on = 1;
    /*  send_sock : la socket qu'on modifie
        IPPROTO_IP: le protocole qu'on modifie
        IP_HDRINCL: option à modifier
        &on: valeur à assigner */

    if(setsockopt(send_sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
        perror("setsockopt IP_HDRINCL");
        return 1;
    };
    // receiving part ------------
    int sock_recv = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock_recv < 0){
        perror("sock_recv");
        return 1;
    };

    char packet_to_send[1515];
    memset(packet_to_send, 0, sizeof(packet_to_send));

    // Header IP et ICMP
    struct iphdr *iph = (struct iphdr *)packet_to_send;
    struct icmphdr *icmph = (struct icmphdr *)(packet_to_send + sizeof(struct iphdr));
    // Payload
    char payload[opts.s];
    bzero(payload, sizeof(payload));
    int payload_len = opts.s;
    // ICMP Header
    // for(uint16_t seq = 0; seq < opts.c; seq++)
    // {
    icmph->type = ICMP_ECHO;
    icmph->code = 0;
    icmph->un.echo.id = htons(my_id);
    icmph->un.echo.sequence = htons(seq); // a mettre dans une boucle plus tard
    // }
    icmph->checksum = 0;
    memcpy(packet_to_send + sizeof(struct iphdr) + sizeof(struct icmphdr), payload, payload_len);
    icmph->checksum = checksum(icmph, sizeof(struct icmphdr) + payload_len);

    // IP HEADER

    iph->ihl = 5; // Internet Header Len
    iph->version = 4; // IPv4
    iph->tos = 0; // Service Type
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + payload_len); // Total len
    iph->id = htons(0); // ID of this packet, no need to be unique, we dont fragment
    iph->frag_off = 0; // Offset fragementation
    iph->ttl = 64; // Time to live , maybe add as bonus ? 
    iph->protocol = IPPROTO_ICMP;
    // iph->check = 0; 
    iph->saddr = inet_addr("0.0.0.0"); // kernel will fill the correct source IP
    iph->daddr = inet_addr(dst_ip); // destination IP
    // iph->check = checksum(iph, sizeof(struct iphdr));

    // Send the packet

    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET; // IPv4
    dest_addr.sin_addr.s_addr = iph->daddr; // Destination IP

    printf("Size payload: %lu bytes\n",
           payload_len);
    printf("Size of icmphdr: %lu bytes\n",
           sizeof(struct icmphdr));
    printf("Size of iphdr: %lu bytes\n",
        sizeof(struct iphdr));

    // Sending packet(s)
    char buf[1500];
    struct timeval tv_send, tv_recv;
    for(uint16_t seq = 0; seq < opts.c_value; seq++)
    {   
        icmph->un.echo.sequence = htons(seq);
        icmph->checksum = 0;
        icmph->checksum = checksum(icmph, sizeof(struct icmphdr) + payload_len);
        gettimeofday(&tv_send, NULL);
        if(sendto(send_sock, packet_to_send, sizeof(struct iphdr) + sizeof(struct icmphdr) +\
               payload_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0){
                perror("sendto");
                return 1;
               }
        // printf("Sent ICMP Echo Request seq=%d to %s\n", seq, dst_ip);
        while(1){
            ssize_t bytes_received = recv(sock_recv, buf, sizeof(buf), 0);
            if(bytes_received < 0){
                perror("recv");
                continue;   
            };
            struct iphdr *recv_iph = (struct iphdr *)buf;
            ssize_t ip_header_len = recv_iph->ihl * 4;
            struct icmphdr *recv_icmph = (struct icmphdr *)(buf + ip_header_len);
            if(recv_icmph->type == ICMP_ECHOREPLY && recv_icmph->un.echo.id == htons(my_id)\
            && recv_icmph->un.echo.sequence == htons(seq)){
                //64 bytes from 8.8.8.8: icmp_seq=0 ttl=63 time=1.778 ms
                gettimeofday(&tv_recv, NULL);
                printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
                    bytes_received - ip_header_len,
                    dst_ip,
                    seq,
                    recv_iph->ttl,
                    (tv_recv.tv_sec - tv_send.tv_sec) * 1000.0 + (tv_recv.tv_usec - tv_send.tv_usec) / 1000.0);
                    if(!opts.flood && seq + 1 < opts.c_value)
                        sleep(atoi(opts.i_is_set ? opts.i : "1"));
                    break;
            };

        }
    };
    close(send_sock);
    close(sock_recv);
    return 0;
}

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


