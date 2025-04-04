// tinystun: A simple STUN client
// Copyright 2024 Chris Hiszpanski. All rights reserved.

// C standard library
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// POSIX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

// STUN header
struct stun_message_header {
    uint16_t type : 14;
    uint16_t reserved : 2;
    uint16_t length;
    uint32_t magic_cookie;
    uint8_t txid[12];
} __attribute__((packed));

void help() {
    printf(
        "usage: tinystun [-6] <host[:port]>\n"
    );
    exit(0);
}

void version() {
    printf(
        "tinystun 1.0\n"
        "Copyright 2024 Chris Hiszpanski. All rights reserved.\n"
    );
    exit(0);
}

int main(int argc, char **argv)
{
    int err;
    int sockfd;

    // sanity check
    if (2 != argc) {
        help();
    }

    // open socket
    if (sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP), -1 == sockfd) {
        perror("socket");
        exit(1);
    }

    // construct request
    struct stun_message_header req = {
        .type = htons(1),
        .length = 0,
        .magic_cookie = htonl(0x2112a442),
    };

    // seed random number generator
    srandom(time(0) ^ (long)&main);

    // generate random transaction id
    for (int i = 0; i < sizeof(req.txid); i++)
        req.txid[i] = (random() & 0xff);

    // split host:port
    char *host;
    int port;
    {
        char *colon = strchr(argv[1], ':');
        if (!colon) {
            host = argv[1];
            port = 3478;
        } else {
            *colon = '\0';
            host = argv[1];
            port = atoi(colon+1);
        }
    }

    // resolve host to address
    struct addrinfo *ai;
    struct addrinfo hint = {
        .ai_family = PF_INET,
        .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED
    };
    if (err = getaddrinfo(host, NULL, &hint, &ai), err) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(1);
    }
    const struct sockaddr_in raddr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = ((struct sockaddr_in *)(ai->ai_addr))->sin_addr
    };
    freeaddrinfo(ai);
 
    // bind to ephemeral port
    if (-1 == connect(sockfd, (const struct sockaddr *)&raddr, sizeof(raddr))) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    // send stun binding request
    if (-1 == send(sockfd, &req, sizeof(req), 0)) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // receive response
    uint8_t buffer[256];
    if (-1 == recv(sockfd, buffer, sizeof(buffer), 0)) {
        perror("recv");
        close(sockfd);
        exit(1);
    }

    // parse and print
    printf("%d.%d.%d.%d:%d\n",
        buffer[28] ^ 0x21,
        buffer[29] ^ 0x12,
        buffer[30] ^ 0xa4,
        buffer[31] ^ 0x42,
        ((buffer[26] ^ 0x21) << 8) | (buffer[27] ^ 0x12)
    );

    // close socket
    if (-1 == close(sockfd)) {
        perror("close");
    }

    return 0;
}
