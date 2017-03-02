#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct msg_echo {
    unsigned short seq;
    unsigned short reserve;
    char msg[32];
};

int main(int argc, char *argv[])
{
    int s, n;
    struct msg_echo echo;
    struct sockaddr_in servskt;
    in_port_t myport;
    
    if (argc != 3) {
        fprintf(stderr, "Usage: ./udpcli localhost port\n");
        exit(1);
    }

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    myport = strtol(argv[2], NULL, 10);
    memset(&servskt, 0, sizeof servskt);
    servskt.sin_family = AF_INET;
    servskt.sin_port = htons(myport);
    inet_aton(argv[1], &servskt.sin_addr);

    if (connect(s, (struct sockaddr *)&servskt, sizeof servskt) < 0) {
        perror("connect");
        exit(1);
    } 

    echo.seq = 0;
    
    for (;;) {
        printf("message: ");
        if (fgets(echo.msg, sizeof echo.msg, stdin) == NULL) {
            break;
        }
        write(s, &echo, sizeof echo);
        if ((n = read(s, &echo, sizeof echo)) < 0) {
            perror("read");
            exit(1);
        }
        printf("seq: %d msg: %s\n", echo.seq, echo.msg);
    }
    close(s);

    return 0;
}
