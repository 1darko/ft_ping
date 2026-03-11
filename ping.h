#ifndef PING_H
#define PING_H

#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <net/ethernet.h>   /* ETH_ALEN */
#include <net/if.h>
#include <net/if_arp.h>     /* ARP hardware types */
#include <netpacket/packet.h> /* sockaddr_ll (AF_PACKET) */
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <limits.h>
#include <signal.h>

#define PAYLOAD_SIZE 56
#define PLAYLOAD_MAX 1473

typedef struct __attribute__((packed)){
   uint8_t dest_addr[ETH_ALEN]; /* Destination hardware address */
   uint8_t src_addr[ETH_ALEN];  /* Source hardware address */
   uint16_t frame_type;   /* Ethernet frame type */
} ether_hdr;

/* Ethernet ARP packet from RFC 826 */
typedef struct __attribute__((packed)){
   uint16_t htype;   /* Format of hardware address */
   uint16_t ptype;   /* Format of protocol address */
   uint8_t hlen;    /* Length of hardware address */
   uint8_t plen;    /* Length of protocol address */
   uint16_t op;    /* ARP opcode (command) */
   uint8_t sha[ETH_ALEN];  /* Sender hardware address */
   uint32_t spa;   /* Sender IP address */
   uint8_t tha[ETH_ALEN];  /* Target hardware address */
   uint32_t tpa;   /* Target IP address */
} arp_ether_ipv4;

typedef struct {
   int v;
   int D;
   char *w;
   int w_value;
   char *c;
   int c_value;
   char *s;
   int s_value;
   int flood;
   char* i;
   int i_is_set;
   int q;
   int r;
   int transmited_packages;
   int received_packages;
   char ipv4[INET_ADDRSTRLEN];
} options;

extern volatile sig_atomic_t stop;

int resolve_ip(const char *name_or_ip, options *opts);
unsigned short checksum(void *b, int len);
int flag_checker(int ac, char **av, options *opts, const char **dst_ip);
int flag_checker2(options *opts);
int int_overflow(char *a);
void value_error(const char *value, char near_char, int type);
int isnumeric(const char *str);
void print_question_mark();
void handle_sigint(int sig);
int ping_loop(int send_sock, int sock_recv, options *opts,
              const char *dst_ip, char *packet,
              struct icmphdr *icmph, struct sockaddr_in *dest_addr,
              uint16_t my_id);
int receive_reply(int sock_recv, options *opts, const char *dst_ip,
                  uint16_t my_id, uint16_t seq, struct timeval tv_send);
void build_packet(char *packet, struct iphdr **iph, struct icmphdr **icmph,
                  options *opts, uint16_t my_id);
int init_sockets(options *opts, int *send_sock, int *sock_recv);

#endif