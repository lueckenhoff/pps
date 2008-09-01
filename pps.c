/*
 * pps.c -- a open a datagram socket and report the packet-per-second(PPS)
 * compile with:	gcc pps.c -lsocket
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef unsigned long long ulonglong;
#define MAXBUFLEN 1600

int main (int argc, char **argv)
{
    char		buf[MAXBUFLEN];
    struct sockaddr_in	my_addr,	/* my address information */
    			their_addr;	/* connector`s address information */
    int			sockfd,
    			addr_len,
    			numbytes;
    ushort		port;
    ulonglong		count;
    fd_set		readfds;
    struct timeval	tv;
    float		pps = 0,
    			weight = 8.0;
    time_t		last = 0,
			t = 0;

    if ((2 != argc) && (3 != argc)) {
	fprintf(stderr, "usage: pps <port> {weight}\n");
	exit(1);
    }
    port = (ushort) strtoul(argv[1], NULL, 0);
    if (3 == argc) {
        weight = atof(argv[2]);
	if (weight < 2.0) {
	    weight = 8.0;
	}
    }
    printf("listening on port %u", port);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(port);     /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* automatically fill with my IP */
    bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */
    if (-1 == bind(sockfd, (struct sockaddr *)&my_addr,
                   sizeof(struct sockaddr))) {
        perror("bind");
        exit(1);
    }
    addr_len = sizeof(struct sockaddr);
    count = 0;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    while (1) {
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	(void) select(sockfd + 1, &readfds, NULL, NULL, &tv);
	if (FD_ISSET(sockfd, &readfds)) {
	    if (-1 == (numbytes = recvfrom(sockfd, buf, MAXBUFLEN, 0,
					   (struct sockaddr *) &their_addr,
					   &addr_len))) {
		perror("recvfrom");
		exit(1);
	    }
	    count++;
	}
	t = time(NULL);
	if (last < t) {
	    pps = ((pps * (weight - 1.0)) + (float) count) / weight;
	    last = t;
	    printf("\n%g pps", pps);
	    fflush(stdout);
	    count = 0;
	}
    }
    close(sockfd);
}
