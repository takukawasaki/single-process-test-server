#include "net_simplelib.h"


int
server_socket(const char *host,const char *portnum){
     char nbuf[NI_MAXHOST], sbuf[NI_MAXHOST];
     struct addrinfo hints, *res0;
     int soc,opt,errcode;
     socklen_t opt_len;
     
     /* zero clear for address info hints */

     bzero(&hints,sizeof(hints));
     hints.ai_family = AF_INET;
     hints.ai_socktype = SOCK_STREAM;
     hints.ai_flags = AI_PASSIVE;
     /*decides address info */

     if ((errcode = getaddrinfo(host,portnum,&hints,&res0)) != 0){
          //(void) fprintf(stderr,"getaddrinfo():%s\n",gai_strerror(errcode));
          err_print("getaddrinfo():%s\n",gai_strerror(errcode));
          return (-1);
     }
     if ((errcode = getnameinfo(res0->ai_addr,res0->ai_addrlen,
                                nbuf,sizeof(nbuf),
                                sbuf,sizeof(sbuf),
                                NI_NUMERICHOST | NI_NUMERICSERV)) != 0){
          (void) err_print("getnameinfo():%s\n",gai_strerror(errcode));
          freeaddrinfo(res0);
          return (-1);
     }


     err_print("port=%s\n",sbuf);
 
     /* make socket */
     if ((soc = socket(res0->ai_family,res0->ai_socktype,res0->ai_protocol)) == -1){

          err_print("socket");
          freeaddrinfo(res0);
          return (-1);
     }

     /*setting for socket option */
     opt = 1;
     opt_len = sizeof(opt);
     if (setsockopt(soc,SOL_SOCKET,SO_REUSEADDR,&opt,opt_len) == -1){

          err_print("setsockopt");
          (void) close(soc);
          freeaddrinfo(res0);
          return (-1);
          
     }

     /* setting socket address */
     if (bind(soc,res0->ai_addr,res0->ai_addrlen) == -1){

          err_print("setsoket");
          (void) close(soc);
          freeaddrinfo(res0);
          return (-1);
          
     }

     /* access back log */
     if (listen(soc,SOMAXCONN) == -1){

          err_print("listen");
          (void) close(soc);
          freeaddrinfo(res0);
          return (-1);
     }
     freeaddrinfo(res0);
     return (soc);
     
          
}
     
void
accept_loop(int soc){
     char hbuf[NI_MAXHOST],sbuf[NI_MAXSERV];
     struct sockaddr_storage from;
     int acc;
     socklen_t len;
     for (;;){
          len = (socklen_t) sizeof(from);
          /* accept connection */
          if ((acc = accept(soc,(struct sockaddr *) &from ,&len)) == -1){
               if (errno != EINTR){
                    err_print("accept");;
                    
               }
          }else {
               (void) getnameinfo((struct sockaddr *) &from ,len,
                                  hbuf,sizeof(hbuf),
                                  sbuf,sizeof(sbuf),
                                  NI_NUMERICHOST | NI_NUMERICSERV);

               err_print("accept:%s:%s\n",hbuf,sbuf);
               
               /*send and receive loop*/
               send_recv_loop(acc);
               /*accept socket close */
               (void) Close(acc);
               acc = 0;
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
          /* response */
          if ((len = send(acc,buf,(size_t) len,0)) == -1){
               /* error */
               err_print("send");
               break;
          }
     }
}


int
main (int argc,char *argv[]){

     int soc;
     /* set port? */
     if (argc <= 1){
          //(void) fprintf(stderr,"server port\n");
          //return (EX_USAGE);
          err_print("server port\n");
     }

     /* prepare server socket */
     if ((soc = server_socket(argv[1],argv[2])) == -1){
          //(void) fprintf(stderr,"server_socket(%s) : error\n",argv[1]);
          //return (EX_UNAVAILABLE);
          err_print("server_socket(%s) : error\n",argv[1]);
          
     }

     //(void) fprintf(stderr,"ready for accept\n");
     err_print("ready for accept\n");
     /*accept loop */
     accept_loop(soc);
     /* socket close */
     (void) Close(soc);

}
