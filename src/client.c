/**
 * client.c
 */
//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif

//#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <inttypes.h> // C99 standard, needed to use printf and scanf with <stdint.h> data types
//#include <stdint.h>   // C99 standard
#include <stdbool.h>  // C99 standard
//#include <wchar.h>    // wchar_t definition
//#include <stddef.h>   // wchar_t definition

//#include <locale.h>

//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

//#include "common.h"

#define MAXDATASIZE 1024

// Usage
void print_usage(const char *name)
{
    printf("Usage: %s [address] [port] [text]\n", name);
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        print_usage(argv[0]);
        exit(1);
    }

    const char *address = argv[1];
    const char *port = argv[2];
    const char *text = argv[3];
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct addrinfo their_addr;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((ret = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "client: (getaddrinfo): %s\n", gai_strerror(ret));
        exit(1);
    }
    
    printf("client: getaddrinfo\n");
    
    int sockfd;

    // loop through the results and get the first socket we can 
    for(struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: (socket)");
            continue;
        }
        
        printf("client: socket %d\n", sockfd);
        
        memcpy(&their_addr, p, sizeof(their_addr));

        break;
    }
    
    freeaddrinfo(servinfo);

    int numbytes;

    if ((numbytes = sendto(sockfd, text, strlen(text), 0, their_addr.ai_addr, their_addr.ai_addrlen)) == -1) {
        perror("client: (sendto)");
        exit(1);
    }

    printf("client: sent %d bytes to %s\n", numbytes, address);
    close(sockfd);

    return 0;
}

