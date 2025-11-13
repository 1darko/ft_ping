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


int resolve_ip(const char *name_or_ip, char *resolved_ip)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    int rc;
    // char ipstr[INET_ADDRSTRLEN];

    if (!name_or_ip || !resolved_ip)
        return -1;

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
        strcpy(resolved_ip, ipstr);
        freeaddrinfo(res);
        return 0;
    }
    return -1;
}

int main(){
    char *entering_adr = "google.com";
    char resolved_ip[INET_ADDRSTRLEN];
    if(resolve_ip(entering_adr, resolved_ip) == 0)
    {

        printf("IP address of %s: %s\n", entering_adr, resolved_ip);
    }
    return(0);

}