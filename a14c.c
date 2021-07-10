#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Limitation
   ・「コマンド ドメイン名 ポート番号」の順に入力する必要がある
*/

/* Global Settings */
enum {CMD_NAME, DST_IP, DST_PORT};

/* Check args */
void check_args(int argc, char *argv[]){
   int i = 0;
   char **p = argv;
   printf("argc = %d\n", argc);
   while(*p){
      printf("argv[%d] = (%s)\n", i, *p);
      i++;
      p++;
   }
}

/* main */
int main(int argc, char *argv[])
{
  int s; /* socket */
  struct sockaddr s_name; /* name for the socket */
  socklen_t namelen;
  int domain;
  int type;
  int protocol;
  struct sockaddr_in server;
  unsigned int ch; /* character read */
  unsigned long dst_ip;    /* ip address of server */
  int port;                /* port number of server */
  char cards[5] = {'1', '2', '3', '4', '5'}; 
  char enemy_cards[5] = {'1', '2', '3', '4', '5'};
  int num_trouble = 5;
  int my_score = 0, enemy_score = 0;
  char selected_card;
  char enemy_selected_card;
  char enemy_bufs[4];
  int i = 0;

   /* Check Args */
   // check_args(argc, argv);
   if(argc != 3){
      printf("Args Error\n");
      return 1;
   }

   /* hostname => ip_address */
   dst_ip = inet_addr(argv[DST_IP]);
   if(dst_ip == -1){
      struct hostent *he;
      he = gethostbyname(argv[DST_IP]);
      if(he == NULL){
         printf("gethostbyname error\n");
         return 1;
      }
      memcpy((char *)&dst_ip, (char *) he->h_addr, he->h_length); 
   }
   // printf("dst_ip = %x\n", dst_ip);
   
   /* servicename => port_number */
   port = atoi(argv[DST_PORT]);
   if(port == 0){
      struct servent *se;
      se = getservbyname(argv[DST_PORT], "tcp");
      if (se != NULL){
         port = (int)ntohs((u_int16_t)se->s_port);
      }else{
         perror("getservbyname");
         return 255;
      }
   }


  /***************************************************/
  /* FILL HERE! */
  /* set domain and type. protocol may be 0 */
   domain = PF_INET; 
   type = SOCK_STREAM; 
   protocol = 0;
  /* Create a socket and set the descriptor to s */
  /* call socket system call. set the return value to 's' */
   s = socket(domain, type, protocol);
   if (s < 0){
      printf("socket error\n");
      return 1;
   }
   // printf("socket success\n");



  /************************************************/

  /* Socket s has been successfully created here */

  /* clear the strucure 'server' */
  memset(&server, 0, sizeof(server));

   server.sin_family = AF_INET;  /* IPv4 */
   server.sin_addr.s_addr = dst_ip; 
   server.sin_port = htons(port);


  /* call connect system call */
  if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0){
     printf("connect error\n");
     return 1;
  }
//   printf("connet success\n");

  /************************************************/

  /* Now the stream s to the server is available.
       We can use it as if it is a file, both for reading and writing.
       Note that it is the matter of application how we read from /write to
       the stream */
  /* For the service "daytime", the client's task is read all
     octets from the server, that is all. */
  /* FILL HERE! */
  /* Read 1 byte from s, while read from s returns 1 as the number of
     bytes read, and print each of character to the terminal */

   /* step [2]  -- wait server message -- */
   char buf;
   unsigned int r;
   printf("(server) : ");
   while(1){
      r = read(s, &buf, 1);
      // printf("r = %d\n",r);
      if(r < 1){
         // printf("break\n");
         break;
      }
      putchar(buf);
      if(buf == '\n'){
         // printf("break\n");
         break;
      }
   }

   /* step [6]  -- check the number of troubles -- */
   while(num_trouble > 1){
      /* step [3]  -- input card -- */
      printf("(client) : cards you own (0 means None): ");
      for(i=0; i<5; i++)printf("%c", cards[i]);
      printf("\n");
      printf("(client) : input card : ");
      // scanf("%[^\n]%*c", sen_buf);
      scanf("%c%*c", &selected_card);
      num_trouble--;

      /* step [4]  -- send massage to server -- */
      char send_buf[4];
      sprintf(send_buf, "%d %c\n", num_trouble, selected_card);
      if(write(s, send_buf, strlen(send_buf)) < 0){
         printf("write error\n");
         return 1;
      }

      /* step [5]  -- read server message -- */
      i = 0;
      printf("(server) : ");
      while(1){
         r = read(s, &buf, 1);
         if(r < 1){
            printf("\n");
            break;
         }
         putchar(buf);
         enemy_bufs[i++] = buf;
         if(buf == '\n'){
            break;
         }
      }
      enemy_selected_card = enemy_bufs[2];
      printf("(client) : enemy_bufs = %s\n", enemy_bufs);
      printf("(client) : enemy_selected_card = %c\n", enemy_selected_card);
      if ((int)selected_card > (int)enemy_selected_card){
         my_score += (int)enemy_selected_card - 48;
      }else if ((int)selected_card < (int)enemy_selected_card){
         enemy_score += (int)selected_card - 48;
      }
      /* used card = 0*/
      cards[(int)selected_card - 48 - 1] = '0';
      enemy_cards[(int)enemy_selected_card - 48 - 1] = '0';
   }

   /* update last score */
   for (i=0; i<5; i++){
      if(cards[i] != '0')selected_card = cards[i];
      if(enemy_cards[i] != '0')enemy_selected_card = enemy_cards[i];
   }
   if ((int)selected_card > (int)enemy_selected_card){
      my_score += (int)enemy_selected_card - 48;
   }else if ((int)selected_card < (int)enemy_selected_card){
      enemy_score += (int)selected_card - 48;
   }
   /* used card = 0 */
   cards[(int)selected_card - 48 - 1] = '0';
   enemy_cards[(int)enemy_selected_card - 48 - 1] = '0';

   /* show game result */
   printf("--------------------------------------\n");
   printf("my score    : %d\nenemy score : %d\n", my_score, enemy_score);
   if(my_score > enemy_score)printf("You win.\n");
   else if(my_score < enemy_score)printf("Enemy win.\n");
   else printf("Draw.\n");

   /* close the socket */
   close(s);

   /* terminate itself with normal status */
   return 0;
}
