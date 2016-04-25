#include "net_simplelib.h"


int
server_socket(const char *host,const char *portnum){
     int listenfd;
     socklen_t addrlen;
     
     listenfd = Tcp_listen(host, portnum, &addrlen);
     return (listenfd);
}


void
accept_loop(int soc){
     char hbuf[NI_MAXHOST],sbuf[NI_MAXSERV];
     struct sockaddr_storage from;
     int connfd;
     socklen_t len;
     
     for (;;){
          len = (socklen_t) sizeof(from);
          /* accept connection */
          if ((connfd = Accept(soc,(struct sockaddr *) &from ,&len)) == -1){
               if (errno != EINTR){
                    err_print("accept");
               }
          }else {
               (void) getnameinfo((struct sockaddr *) &from ,len,
                                  hbuf,sizeof(hbuf),
                                  sbuf,sizeof(sbuf),
                                  NI_NUMERICHOST | NI_NUMERICSERV);

               err_print("accept:%s:%s\n",hbuf,sbuf);
               
               /*send and receive loop*/
               send_recv_loop(connfd);
               /*accept socket close */
               Close(connfd);
               connfd = 0;
          }
     }
}


/* receive and sent loop */

void
send_recv_loop(int acc){
     char buf[512],*ptr;
     ssize_t len;
     for (;;){
          /*receive*/
          if ((len = recv(acc,buf,sizeof(buf),0)) == -1){
               /*erro */
               //perror("recv");
               err_print("recv");
               break;
          }
          if (len == 0){
               /*end of file */
               //(void) fprintf(stderr,"recv:EOF\n");
               err_print("recv:EOF\n");
               break;
          }
          /* strings make */
          buf[len] = '\0';
          if ((ptr = strpbrk(buf,"\r\n")) != NULL) {
               *ptr = '\0';
          }
          //(void) fprintf(stderr,"[client]%s\n",buf);
          err_print("[client]%s\n",buf);

          
          /* making response strings */
          (void)mystrlcat(buf,":OK\r\n",sizeof(buf));
          len = (ssize_t) strlen(buf);
          
          Send(acc,buf,(size_t)len,0);
          
          /* if ((len = send(acc,buf,(size_t) len,0)) == -1){ */
          /*      /\* error *\/ */
          /*      err_print("send"); */
          /*      break; */
          /* } */

     }
}



int
main (int argc,char *argv[]){

     int soc;
     /* set port? */
     if (argc < 3){
          err_quit("usage: ./server <host address> <port>\n");
     }
     
     /* prepare server socket */
     if ((soc = server_socket(argv[1],argv[2])) == -1){
          err_print("server_socket(%s) : error\n",argv[1]);
     }

     err_print("ready for accept\n");
     /*accept loop */
     accept_loop(soc);
     /* socket close */
     Close(soc);

}
