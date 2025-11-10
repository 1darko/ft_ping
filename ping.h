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

#define PAYLOAD_SIZE 56
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
   int w;
   int c;
   int s;
   int flood;
   char* i;
   int i_is_set;

} options;

#endif