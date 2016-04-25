#include "net_simplelib.h"

int
Socket(int family, int type, int protocol)
{
     int n;
     if ((n = socket(family, type, protocol)) < 0) {
          err_sys("socket error");

     }
     return(n);     
}


void
Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
     if (bind(fd, sa, salen) < 0) {
          err_sys("bind error");

     }
}


void
Connect(int fd, const struct sockaddr  *sa, socklen_t salen)
{
     
     if (connect(fd, sa, salen) < 0) {
          err_sys("connect error");
     }
}

int
Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
     int		n;

again:
     if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
          if (errno == EPROTO || errno == ECONNABORTED)
#else
               if (errno == ECONNABORTED)
#endif
                    goto again;
               else
                    err_sys("accept error");
     }
     return(n);
}

/* include Listen */
void
Listen(int fd, int backlog)
{
     char	*ptr;

     /*4can override 2nd argument with environment variable */
     if ( (ptr = getenv("LISTENQ")) != NULL)
          backlog = atoi(ptr);

     if (listen(fd, backlog) < 0)
          err_sys("listen error");
}

/* end Listen */

void
Listen2(int fd, int backlog,struct addrinfo *res)
{
     char	*ptr;

     /*4can override 2nd argument with environment variable */
     if ( (ptr = getenv("LISTENQ")) != NULL)
          backlog = atoi(ptr);

     if (listen(fd, backlog) < 0){
          err_print("listen error");
          Close(fd);
          freeaddrinfo(res);
     }
  
}
/* end Listen */


void
Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
     if (getpeername(fd, sa, salenptr) < 0) {
          err_sys("getpeername error");
     }
}

void
Getsockname(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
     if (getsockname(fd,sa, salenptr) < 0) {
          err_sys("getsockname error");
     }
}

void
Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr)
{
     if (getsockopt(fd, level, optname, optval, optlenptr) < 0){
          err_sys("getsockopt error");
     }
}

#ifdef	HAVE_POLL
int
Poll(struct pollfd *fdarray, unsigned long nfds, int timeout)
{
     int		n;

     if ( (n = poll(fdarray, nfds, timeout)) < 0)
          err_sys("poll error");

     return(n);
}
#endif


ssize_t
Recv(int fd, void *ptr, size_t nbytes, int flags)
{
     ssize_t n;
     if ((n = recv(fd, ptr, nbytes, flags)) < 0) {
          err_sys("recv error");
     }
     return(n);
}


ssize_t
Recvfrom(int fd, void *ptr, size_t nbytes, int flags,
         struct sockaddr *sa, socklen_t *salenptr)
{
     ssize_t n;
     if ((n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0) {
          err_sys("recvfrom error");
     }
     return(n);
}


ssize_t
Recvmsg(int fd, struct msghdr *msg, int flags)
{
     ssize_t n;
     if ((n = recvmsg(fd, msg, flags)) < 0) {
          err_sys("recvmsg error");
     }
     return(n);
     
}

int
Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout)
{
     int		n;

     if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
          err_sys("select error");
     return(n);		/* can return 0 on timeout */
}




void
Sendto(int fd, const void *ptr, size_t nbytes, int flags,
       const struct sockaddr *sa, socklen_t salen)
{
     if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes)
          err_sys("sendto error");
}

void
Sendmsg(int fd, const struct msghdr *msg, int flags)
{
     unsigned int	i;
     ssize_t			nbytes;

     nbytes = 0;	/* must first figure out what return value should be */
     for (i = 0; i < msg->msg_iovlen; i++)
          nbytes += msg->msg_iov[i].iov_len;

     if (sendmsg(fd, msg, flags) != nbytes)
          err_sys("sendmsg error");
}

void
Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
     if (setsockopt(fd, level, optname, optval, optlen) < 0)
          err_sys("setsockopt error");
}

void
Shutdown(int fd, int how)
{
     if (shutdown(fd, how) < 0)
          err_sys("shutdown error");
}

int
Sockatmark(int fd)
{
     int		n;

     if ( (n = sockatmark(fd)) < 0)
          err_sys("sockatmark error");
     return(n);
}


void
Socketpair(int family, int type, int protocol, int *fd)
{
     int		n;

     if ( (n = socketpair(family, type, protocol, fd)) < 0)
          err_sys("socketpair error");
}


int Tcp_listen(const char *host, const char *service, socklen_t *addrlenp)
{
     char nbuf[NI_MAXHOST], sbuf[NI_MAXHOST];
     int listenfd, n;
     const int on = 1;
     struct addrinfo hints, *res, *ressave;
     bzero(&hints,sizeof(struct addrinfo));
     hints.ai_flags = AI_PASSIVE;
     hints.ai_family = AF_UNSPEC;
     hints.ai_socktype = SOCK_STREAM;

     if ((n = getaddrinfo(host, service, &hints, &res)) != 0) {
          err_quit("tcp_listen error for %s, %s: %s",
                   host, service,gai_strerror(n));
     }

     if ((n = getnameinfo(res->ai_addr,res->ai_addrlen,
                          nbuf,sizeof(nbuf),
                          sbuf,sizeof(sbuf),
                          NI_NUMERICHOST | NI_NUMERICSERV)) != 0){
          err_print("getnameinfo():%s\n",gai_strerror(n));
          freeaddrinfo(res);
          return (-1);
     }
     err_print("port=%s\n",sbuf);
     ressave = res;

     do
     {
          listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
          if (listenfd < 0) {
               err_print("listen");

               freeaddrinfo(res);
               continue;
          }
          Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

          if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0) {
               break;
          }
          Close(listenfd);
          freeaddrinfo(res);
          
     } while ((res = res->ai_next) != NULL);
     

     if (res == NULL) {
          err_sys("tcplisten error for %s, %s", host, service);
     }
     
     Listen(listenfd, LISTENQ);
     
     if (addrlenp) {
          *addrlenp = res->ai_addrlen;
     }
     
     freeaddrinfo(ressave);

     return(listenfd);
}




#ifdef	HAVE_SOCKADDR_DL_STRUCT
#include	<net/if_dl.h>
#endif

/* include sock_ntop */
char *
sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
     char		portstr[8];
     static char str[128];		/* Unix domain is largest */

     switch (sa->sa_family) {
     case AF_INET: {
          struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

          if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
               return(NULL);
          if (ntohs(sin->sin_port) != 0) {
               snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
               strcat(str, portstr);
          }
          return(str);
     }
/* end sock_ntop */

#ifdef	IPV6
     case AF_INET6: {
          struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

          str[0] = '[';
          if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL)
               return(NULL);
          if (ntohs(sin6->sin6_port) != 0) {
               snprintf(portstr, sizeof(portstr), "]:%d", ntohs(sin6->sin6_port));
               strcat(str, portstr);
               return(str);
          }
          return (str + 1);
     }
#endif

#ifdef	AF_UNIX
     case AF_UNIX: {
          struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

          /* OK to have no pathname bound to the socket: happens on
             every connect() unless client calls bind() first. */
          if (unp->sun_path[0] == 0)
               strcpy(str, "(no pathname bound)");
          else
               snprintf(str, sizeof(str), "%s", unp->sun_path);
          return(str);
     }
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
     case AF_LINK: {
          struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;

          if (sdl->sdl_nlen > 0)
               snprintf(str, sizeof(str), "%*s (index %d)",
                        sdl->sdl_nlen, &sdl->sdl_data[0], sdl->sdl_index);
          else
               snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
          return(str);
     }
#endif
     default:
          snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
                   sa->sa_family, salen);
          return(str);
     }
     return (NULL);
}

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
     char	*ptr;

     if ( (ptr = sock_ntop(sa, salen)) == NULL)
          err_sys("sock_ntop error");	/* inet_ntop() sets errno */
     return(ptr);
}



pid_t
Fork(void)
{
     pid_t	pid;

     if ( (pid = fork()) == -1)
          err_sys("fork error");
     return(pid);
}


Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
     Sigfunc	*sigfunc;

     if ( (sigfunc = signal(signo, func)) == SIG_ERR)
          err_sys("signal error");
     return(sigfunc);
}

void
Close(int fd)
{
     if (close(fd) == -1)
          err_sys("close error");
}

void *
Malloc(size_t size)
{
     void	*ptr;

     if ( (ptr = malloc(size)) == NULL)
          err_sys("malloc error");
     return(ptr);
}




int
tcp_connect(const char *host, const char *service)
{
     char nbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
     int				sockfd, n;
     struct addrinfo	hints, *res, *ressave;

     bzero(&hints, sizeof(struct addrinfo));
     hints.ai_family = AF_UNSPEC;
     hints.ai_socktype = SOCK_STREAM;

     if ( (n = getaddrinfo(host, service, &hints, &res)) != 0)
          err_quit("tcp_connect error for %s, %s: %s",
                   host, service, gai_strerror(n));

     if ((n = getnameinfo(res->ai_addr,res->ai_addrlen,
                          nbuf,sizeof(nbuf),
                          sbuf,sizeof(sbuf),
                          NI_NUMERICHOST | NI_NUMERICSERV)) != 0){
          err_print("getnameinfo():%s\n",gai_strerror(n));
          freeaddrinfo(res);
          return (-1);
     }
     err_print("addr=%s\n", nbuf);
     err_print("port=%s\n", sbuf);
     
     ressave = res;

     do {
          sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
          if (sockfd < 0){
               err_print("socket");
               freeaddrinfo(res);
               continue;	/* ignore this one */
          }
          if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
               break;		/* success */

          Close(sockfd);	/* ignore this one */
          freeaddrinfo(res);
          
     } while ( (res = res->ai_next) != NULL);

     if (res == NULL)	/* errno set from final connect() */
          err_sys("tcp_connect error for %s, %s", host, service);

     freeaddrinfo(ressave);
     return(sockfd);
}
/* end tcp_connect */

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_connect() function.
 */

int
Tcp_connect(const char *host, const char *service)
{
     return(tcp_connect(host, service));
}
