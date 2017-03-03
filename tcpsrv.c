#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

struct msg_echo {
    unsigned short seq;
    unsigned short reserve;
    char msg[32];
};

int main(int argc, char *argv[])
{
    int s, s2, n;
    pid_t pid;
    socklen_t recvlen;
    in_port_t myport;
    struct sockaddr_in servskt, recvskt;
    struct msg_echo echo;
    
    if (argc != 2) {
        fprintf(stderr, "Usage: ./udpsrv port\n");
        exit(1);
    }
    
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    myport = strtol(argv[1], NULL, 10);
    memset(&servskt, 0, sizeof servskt);
    servskt.sin_family = AF_INET;
    servskt.sin_port = htons(myport);
    servskt.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&servskt, sizeof servskt) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(s, 5) != 0) {
        perror("listen");
        exit(1);
    }
    
    for (;;) {
        recvlen = sizeof recvskt;
        if ((s2 = accept(s, (struct sockaddr *)&recvskt, &recvlen)) < 0) {
            perror("accept");
            exit(1);
        }

        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            for(;;) {
                if ((n = recv(s2, &echo, sizeof echo, 0)) < 0) {
                    perror("recv");
                    exit(1);
                }
                printf("%s\n", echo.msg);
                echo.seq++;
                if ((n = send(s2, &echo, sizeof echo, 0)) < 0) {
                    perror("send");
                    exit(1);
                }
                if (strcmp(echo.msg, "FIN") == 0 || echo.seq == 10) {
                    printf("Finished\n");
                    break;
                }
            }
            close(s2);
            exit(1);
        }
        close(s2);
    }
    close(s);

    return 0;
}
