#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define SERV_PORT 43193 /* <-- SET your assinged port number !! */


void decompuint32(unsigned int x, unsigned char y[4], int hn){
    if(hn != 0)x = ntohl(x);

    int i;
    for(i=3; i>=0; i--){
        y[i] = x % 0x100;
	x /= 0x100;
    }
}


void dotdec(unsigned int x, char *buf, int hn){
    unsigned char y[4];
    long s[4];
    int j;
    decompuint32(x, y, hn);
    sprintf(buf, "%ld.%ld.%ld.%ld", y[0], y[1], y[2], y[3]);
}


void read_1_octet(char* buf, int socket){
    char ch;
    unsigned int r;
    int i = 0;
    while(1){
        r = read(socket, &ch, 1);
        if(r < 1){
            printf("read error\n");
            return 255;
        }else{
            putchar(ch);
            buf[i++] = ch;
            if (ch == '\n'){
                break;
            }
        }
    }
}


int main(int argc, char *argv[])
{
    int s; /* socket */
    int s0, s1; /* socket used for each client*/
    struct sockaddr s_name; /* name for the socket */
    socklen_t namelen;
    int domain;
    int type;
    int protocol;
    struct sockaddr_in server;
    struct sockaddr_in client_0;
    struct sockaddr_in client_1;
    u_int addrlen;
    u_short port = SERV_PORT;
    int backlog; /* how many connections may be queued to accept */
    unsigned char ch; /* character to write */
    char greeting_0[32] = "hello "; 
    char greeting_1[32] = "hello ";
    char *cp;


    /*----- (1) Socket -----*/

    domain = PF_INET;           /* Internet version 4 protocols */
    type   = SOCK_STREAM;       /* Stream socket */
    protocol = 0;

    /* Create a socket and set the descriptor to s */

    if ( (s = socket(domain, type, protocol)) == -1) {
        perror("socket");
        return(255);
    }
    /* Socket s has been successfully created here */
 
    /*----- (2) Bind --- You have to code HERE vvv ---*/

    memset( &server, 0, sizeof( server)); /* clear the structure */

    /* SET the contents of server structure appropriately.
     * Note that server.sin_addr.s_addr should be 133.44.78.101
     * (in network order 4 octets)
     * server.sin_port should be "port" you defined.
     * and server.sin_family should be AF_INET
     */
    server.sin_family       = AF_INET;              /* AF_INET */
    // server.sin_addr.s_addr  = htonl(0x852c4e65);    /* ip address (133.44.78.101) */
    // server.sin_addr.s_addr  = htonl(0x852c7564);    /* ip address (133.44.117.100) */
    server.sin_addr.s_addr  = htonl(INADDR_ANY); 
    server.sin_port         = htons(port);                 /* service port */
    if (bind(s, (struct sockaddr *)&server, (socklen_t)sizeof(server)) == -1) {
        perror("bind");
        return(255);
    }    

    /*-------------------------------------- ^^^ ---------*/

    printf("Bound\n");

    /*----- (3) Listen -- You have to code HERE vvv ---*/
    /* listen the socket s, with an appropriate value of backlog */
    backlog = 5;
    if(listen(s, backlog) < 0){
        perror("listen");
        return(255);
    }
    /*--------------------------------------- ^^^ -*/

    printf("Listenning\n");

    unsigned int r0, r1;

    /* Accept & Process Connection loop */
    for (;;) {
        /*----- (4) Accept  -----*/
        /* step [1]  -- accept 2 clients -- */

        /* 1st client accept */
        addrlen = sizeof(client_0);
        s0 = accept(s, (struct sockaddr *)&client_0, (socklen_t *)&addrlen);
        if(s0 < 0){
            perror("accept");
            return(255);
        }
        printf( "Client_0 IP Address: 0x%x\n", ntohl(client_0.sin_addr.s_addr));
        printf( "Client_0 Port: %d\n", ntohs(client_0.sin_port));
        printf("Accepted\n");
        
        addrlen = sizeof(client_1);
        /* 2nd client accept */
        s1 = accept(s, (struct sockaddr *)&client_1, (socklen_t *)&addrlen);
        if(s1 < 0){
             perror("accept");
            return(255);
        }
        printf( "Client_1 IP Address: 0x%x\n", ntohl(client_1.sin_addr.s_addr));
        printf( "Client_1 Port: %d\n", ntohs(client_1.sin_port));
        printf("Accepted\n");

        printf("s0 = %d, s1 = %d\n", s0, s1);
       /*------------------------------------------ ^^^ ---*/

        /* step [2]  -- send first message to 2 clients -- */
        char buf_0[16];
        char buf_1[16];
        dotdec(ntohl(client_0.sin_addr.s_addr), buf_0, 0);
        sprintf(greeting_0, "5 ready %s:%d\n", buf_0, ntohs(client_0.sin_port));
        printf("%s\n", greeting_0);
        if(write(s1, greeting_0, strlen(greeting_0)) < 0){
            break;
        }
        dotdec(ntohl(client_1.sin_addr.s_addr), buf_1, 0);
        sprintf(greeting_1, "5 ready %s:%d\n", buf_1, ntohs(client_1.sin_port));
        printf("%s\n", greeting_1);
        if(write(s0, greeting_1, strlen(greeting_1)) < 0){
            break;
        }

        while(1){
            /* step [3]  -- recv message from 2 clients -- */
            char rec_buf_0[4];
            char rec_buf_1[4];
            printf("waiting messages from client_0 and client_1\n");
            if (r0 = read(s0, &rec_buf_0, 4) < 0){
                printf("r0 break\n");
                break;
            }
            if (r1 = read(s1, &rec_buf_1, 4) < 0){
                printf("r1 break\n");
                break;
            }
            printf("client_0 : %s", rec_buf_0);
            printf("client_1 : %s", rec_buf_1);

            /* step [4]  -- if 1st character of client message is "0", close connection -- */
            if(rec_buf_0[0] == '0' || rec_buf_1[0] == '0'){
                close(s0);
                close(s1);
                printf("all connections closed.\n");
                break;

            /* step [5]  -- send message of client_0(client_1) to client_1(client_0) -- */
            }else{
                if(write(s0, rec_buf_1, strlen(rec_buf_1)) < 0)break;
                if(write(s1, rec_buf_0, strlen(rec_buf_0)) < 0)break;
                printf("send massage to clients\n");
            }

            /* step [6]  -- wait client message -- */

            /* check the number of clients */
            if (s0 < 0 || s1 < 0){
                close(s0);
                close(s1);
                break;
            }
        }
    }
    /* close the socket */
    close(s);

    /* terminate itself with normal status */
    return 0;
}
