/**
 * server.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <inttypes.h>   // C99 standard
//#include <stdint.h>   // C99 standard
#include <stdbool.h>    // C99 standard
//#include <wchar.h>    // wchar_t definition
//#include <stddef.h>   // wchar_t definition

#include <errno.h>

#include <signal.h>
#include <sys/wait.h>

//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "common.h"

#define LISTEN_BACKLOG 20
#define MAX_BUFFER_SIZE 1024

// Usage
void print_usage(const char *name)
{
    printf("Usage: %s [address] [port]\n", name);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        print_usage(argv[0]);
        exit(1);
    }

    const char *address = argv[1];
    const char *port = argv[2];
    struct addrinfo hints;
    struct addrinfo *servinfo;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((ret = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "server: (getaddrinfo): %s\n", gai_strerror(ret));
        exit(1);
    }
        
    printf("server: getaddrinfo\n");
    
    bool is_bound = false;
    int sockfd;

    // loop through the results and bind to the first one that we can
    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: (socket)");
            continue;
        }

        printf("server: socket %d\n", sockfd);

        // avoid "Address already in use" error message 
        if (enable_reuse_addr(sockfd) == -1) {
            perror("server: (setsockopt)");
            exit(1);
        }
        
        printf("server: setsockopt SO_REUSEADDR yes\n");

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: (bind)");
            continue;
        }

        // socket created and bound ... then print info and break
        char ipstr[INET6_ADDRSTRLEN];
        uint16_t ipport;

        inet_ntop(p->ai_family, get_sockaddr_addr(p->ai_addr), ipstr, sizeof(ipstr));
        ipport = ntohs(get_sockaddr_port(p->ai_addr));
        printf("server: bind %s %u\n", ipstr, ipport);
    
        is_bound = true;
        
        break;
    }

    freeaddrinfo(servinfo);

    if (!is_bound) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof(their_addr);
    char buffer[MAX_BUFFER_SIZE];
    int numbytes;
    if ((numbytes = recvfrom(sockfd, buffer, (MAX_BUFFER_SIZE - 1), 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("server: (recvfrom)");
        exit(1);
    }

    char client_address[INET6_ADDRSTRLEN];
    uint16_t client_port = ntohs(get_sockaddr_port((struct sockaddr *)&their_addr));

    inet_ntop(their_addr.ss_family, get_sockaddr_addr((struct sockaddr *)&their_addr), client_address, sizeof(client_address));

    printf("server: got packet from %s %u\n", client_address, client_port); 
    printf("server: packet is %d bytes long\n", numbytes);
    buffer[numbytes] = '\0';
    printf("server: packet contains \"%s\"\n", buffer);

    close(sockfd);

    return 0;
}

