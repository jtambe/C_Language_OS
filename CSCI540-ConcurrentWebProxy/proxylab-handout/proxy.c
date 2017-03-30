/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Andrew Carnegie, ac00@cs.cmu.edu
 *     Harry Q. Bovik, bovik@cs.cmu.edu
 *
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 * Jayesh Tambe
 */

#include "csapp.h"


// 2 Mutex
static sem_t client_sock_mutex;
static sem_t proxy_log_mutex;

// pointer to log file
FILE *proxy_log;


// operation properties
struct operation
{
  int fd;
  struct sockaddr_in sockaddr;
};

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
//added
void doit( int fd, struct sockaddr_in sockaddr);
void read_header_request(rio_t *rp, char *data, int *length, int *chunked);
void *thread(void *vargp);
int open_clientfd_ts(char *hostname, int port);
int parse_header(char *block_header);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);


// Wrappers on top of Robust I/O functions
void Rio_writen_w(int fd, void *usrbuf, size_t n)
{
  if (rio_writen(fd, usrbuf, n) != n)
  {
    fprintf(stderr, "Rio_writen_w: %s\n", strerror(errno));
  }
}

ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n)
{
  ssize_t err;
  if ((err = rio_readnb(rp, usrbuf, n)) < 0)
  {
    fprintf(stderr, "Rio_readnb_w error: %s\n", strerror(errno));
    return 0;
  }
  return err;
}

ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen)
{
  ssize_t err;
  if ((err = rio_readlineb(rp, usrbuf, maxlen)) < 0)
  {
    fprintf(stderr, "Rio_readlineb_w error: %s\n", strerror(errno));
    return 0;
  }
  return err;
}




/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv)
{
    int listenfd, connfd ,port;
    socklen_t clientlen;
    pthread_t tid;
    /* a structure to contain an internet address defined in the include file in.h
    struct sockaddr_in
    {
      short   sin_family;  //should be AF_INET
      u_short sin_port;
      struct  in_addr sin_addr;
      char    sin_zero[8]; //not used, must be zero
    };*/
    struct sockaddr_in clientaddr;


    /* Check arguments */
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	exit(0);
    }

    // get port number as integer value
    port = atoi(argv[1]);
    // initialize binary semaphores
    Sem_init(&client_sock_mutex,0,1);
    Sem_init(&proxy_log_mutex,0,1);
    //ignore SIGPIPE signal
    Signal(SIGPIPE, SIG_IGN);

    //open log file in append mode
    proxy_log = fopen("proxy.log","a");

    // open client socket for listining
    listenfd = Open_listenfd(port);
    printf("Proxy server is listening....\n");
    while(1)
    {
      // get length to pass to accept function
      clientlen = sizeof(clientaddr);
      // get connection file descriptor
      //extract the first connection on the queue of pending connections
      //create a new socket with the same socket type protocol and address family as the specified socket,
      //and allocate a new file descriptor for that socket
      connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
      // allocate new memory for operation
      struct operation *vargp = (struct operation *) Malloc(sizeof(struct operation));
      vargp->fd = connfd;  // set file descriptor to struct pointer fd
      vargp->sockaddr = clientaddr;
      //create thread
      //thread is the start_routine()
      Pthread_create(&tid, NULL, thread, vargp);
    }


    exit(0);
}


/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
	hostname[0] = '\0';
	return -1;
    }

    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* Extract the port number */
    *port = 80; /* default */
    if (*hostend == ':')
	*port = atoi(hostend + 1);

    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
	pathname[0] = '\0';
    }
    else {
	pathbegin++;
	strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
		      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /*
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s", time_str, a, b, c, d, uri);
}


// detaching threads to reclaim memory resources once it terminates
void * thread(void *vargp)
{
  //when detached, thread terminates,
  //its resources are automatically released back to the system
  //without the need for another thread to join with the terminated thread
  Pthread_detach(pthread_self());
  struct operation * thread_opr = (struct operation*) vargp;
  // handle http transaction
  doit(thread_opr->fd, thread_opr->sockaddr);
  // closes file descriptor
  close(thread_opr->fd);
  // wrapper around free from csapp.c
  Free(vargp);
  return NULL;
}

//http transaction handler
void doit(int fd, struct sockaddr_in sockaddr)
{
  rio_t rio_server_fd, rio_client_fd;
  int data_length, block_length, block_encode, port , serverfd, size = 0;
  char hostname[MAXLINE] ,pathname[MAXLINE], buf[MAXLINE];
  char uri[MAXLINE],version[MAXLINE], logstring[MAXLINE], method[MAXLINE];
  char headers[MAXBUF],request[MAXBUF], response[MAXBUF];


  // this will set up empty read buffer and attach an open file descriptor with it
  Rio_readinitb(&rio_client_fd, fd);

  // copy from read buffer to user buffer and return number of bytes copied
  // if number of bytes <= 0 then return
  if (Rio_readlineb_w(&rio_client_fd, buf, MAXLINE) <= 0)
  {return;}

  // read data from buffer to further analyze
  sscanf(buf, "%s %s %s", method, uri, version);
  // if it is not get/post method strcmp will both return non zero values
  if(strcmp(method, "GET") && strcmp(method, "POST") )
  {
    clienterror(fd, uri, "501", "Web Proxy Error", "Web Proxy has not implemented this method");
    return;
  }
  read_header_request(&rio_client_fd, headers, &data_length, &block_encode);

  if(parse_uri(uri,hostname,pathname,&port) == -1)
  {
    clienterror(fd, uri, "502","Web Proxy Error","Web Proxy couldn't find this uri");
    return;
  }

  // building http request
  sprintf(request, "%s /%s %s\r\n%s", method, pathname,version,headers);

  //send http request to web server
  if((serverfd = open_clientfd_ts(hostname,port)) == -1)
  {
    return;
  }
  Rio_writen_w(serverfd, request, strlen(request));
  if(strcmp(method, "POST") == 0)
  {
    Rio_readnb_w(&rio_client_fd, buf, data_length);
    Rio_writen_w(serverfd, buf, data_length);
  }

  // get header of response
  Rio_readinitb(&rio_server_fd, serverfd);
  read_header_request(&rio_server_fd, response, &data_length, &block_encode);

  // send http response to client
  Rio_writen_w(fd, response, strlen(response));


  // send the response content to client
  if(block_encode)
  {
      // read textline from filedscriptor into buffer
    	Rio_readlineb_w(&rio_server_fd, buf, MAXLINE);
      // write length of bytes from buffer to filedescriptor
    	Rio_writen_w(fd, buf, strlen(buf));
    	while((block_length = parse_header(buf)) > 0)
      {
        size += block_length;
        // read block length bytes from filedescriptor into buffer
        Rio_readnb_w(&rio_server_fd, buf, block_length);
        // write length of bytes from buffer to filedescriptor
        Rio_writen_w(fd, buf, block_length);
        // read textline from filedescriptor into buffer
        Rio_readlineb_w(&rio_server_fd, buf, MAXLINE);
        // write length of bytes from buffer to filedescriptor
        Rio_writen_w(fd, buf, strlen(buf));
        // read textline from filedescriptor into buffer
        Rio_readlineb_w(&rio_server_fd, buf, MAXLINE);
        // write length of bytes from buffer to filedescriptor
        Rio_writen_w(fd, buf, strlen(buf));
    	}
      // read textline from filedescriptor into buffer
    	Rio_readlineb_w(&rio_server_fd, buf, MAXLINE);
      // write length of bytes from buffer to filedescriptor
    	Rio_writen_w(fd, buf, strlen(buf));
  }
  else if(data_length > 0)
  {
     size += data_length;
     int temp_length = data_length;
     int curr_length = 0;
     while(temp_length > 0)
     {
       curr_length = (temp_length > MAXBUF ? MAXBUF : temp_length);
       temp_length -= curr_length;
       // read block length bytes from filedescriptor into buffer
       Rio_readnb_w(&rio_server_fd, buf, curr_length);
       // write length of bytes from buffer to filedescriptor
       Rio_writen_w(fd, buf, curr_length);
     }
  }
  else
  {
    // read textline from filedescriptor into buffer
    while((block_length = Rio_readlineb_w(&rio_server_fd, buf, MAXBUF)) > 0)
    {
      size += block_length;
      // write length of bytes from buffer to filedescriptor
      Rio_writen_w(fd, buf, block_length);
    }
  }

  //write to proxy.log file
  // use lock
  //P(&proxy_log_mutex);
  sem_wait(&proxy_log_mutex);
  format_log_entry(logstring, &sockaddr, uri, size);
  // entering records in log file
  fprintf(proxy_log, "%s\n", logstring);
  // release lock
  //V(&proxy_log_mutex);
  sem_post(&proxy_log_mutex);
  //close filedescriptor
  close(serverfd);

}


void read_header_request(rio_t *rp, char *data, int *length, int *chunked)
{
   *length = *chunked = 0;
   char buf[MAXLINE];

   // readline with a wrapper
   Rio_readlineb_w(rp, buf, MAXLINE);
   // copy buffer info into data
   strcpy(data, buf);
   // append connection close to data
   strcat(data, "Connection: close\r\n");
   // while there is content to read
   while(strcmp(buf, "\r\n"))
   {
      // read a line
      Rio_readlineb_w(rp, buf, MAXLINE);
      if(strncasecmp(buf, "Content-Length:", 15) == 0)
      {
        // get length of content
        *length = atoi(buf + 15);
      }
      if(strncasecmp(buf, "Transfer-Encoding: chunked", 26) == 0)
      {
        // if buffer has transfer encoding chunked
        *chunked = 1;
      }
      //if header contains Proxy-Connection: or connection field then we have to keep connection alive
      if(strncasecmp(buf, "Proxy-Connection:", 17) == 0 || strncasecmp(buf, "Connection:", 11) == 0)
      {
        continue;
      }
      // append buf to data
      strcat(data, buf);
   }
}

// returns block lenght of encoded-chunked header
int parse_header(char *block_header)
{
	char ch;
	int i, length = 0;
	for(i = 0; (ch = block_header[i]) != '\r'; i++)
  {
    if(isdigit(ch))
    {
      length = length*16 + ch - '0';
    }
		else if(ch >= 'A' && ch <= 'F')
    {
      length = length*16 + ch - 'A' + 10;
    }
    else if(ch >= 'a' && ch <= 'f')
    {
      length = length*16 + ch - 'a' + 10;
    }
		else
    {
      return -1;
    }
  }
	return length;
}


// open connection to server at hostname and port
// semaphore is used to make it thread safe
int open_clientfd_ts(char *hostname, int port)
{
  int clientfd;
  // gethostbyname to be used on hostent structure to get host name
  /*
  struct hostent
  {
    char *h_name;       // official name of host
    char **h_aliases;   // alias list
    int h_addrtype;     // host address type
    int h_length;       // length of address
    char **h_addr_list; // list of addresses
  };
  */
  struct hostent *hp, *copyp;
  struct sockaddr_in serveraddr;

  if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    return -1;
  }
  // fill in server's IP address and port
  //P(&client_sock_mutex);
  sem_wait(&client_sock_mutex);
  if((copyp = gethostbyname(hostname)) == NULL)
  {
	  //V(&client_sock_mutex);
    sem_post(&client_sock_mutex);
	  return -2;
  }
  hp = (struct hostent*) malloc(sizeof(struct hostent));
  // void *memcpy(void *destination, const void *source, size_t length)
  memcpy(hp, copyp, sizeof(struct hostent));
  //V(&client_sock_mutex);
  sem_post(&client_sock_mutex);
  //void bzero(char *s, int n);
  //bzero((char *) &serveraddr, sizeof(serveraddr)); // bzero is deprecated version of memset
  //void *memset(void *s, int c, size_t n);
  // sets c in s, n = number of bytes to be set
  memset((void *)&serveraddr, 0, sizeof(serveraddr));
  /* a structure to contain an internet address defined in the include file in.h
  struct sockaddr_in
  {
    short   sin_family;  //should be AF_INET
    u_short sin_port;
    struct  in_addr sin_addr;
    char    sin_zero[8]; //not used, must be zero
    http://stackoverflow.com/questions/1593946/what-is-af-inet-and-why-do-i-need-it
  };
  struct in_addr
  {
    uint32_t s_addr;     //address in network byte order
  };*/
  serveraddr.sin_family = AF_INET; // family of IPv4. AF_INET6 = family of IPv6
  //bcopy((char *)hp->h_addr_list[0],(char *)&serveraddr.sin_addr.s_addr, hp->h_length);
  // void *memcpy(void *destination, const void *source, size_t length)
  memcpy((void *)&serveraddr.sin_addr.s_addr,(void *)hp->h_addr_list[0], hp->h_length);
  // htons is conversion from host to network
  // it converts little endian format to big endian format for network traffic
  serveraddr.sin_port = htons(port);
  free(hp);

  if(connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
  {return -1;}

  return clientfd;
}


// clienterror function reference : page 922 Computer Systems Programmers perspective
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{

 char buf[MAXLINE], body[MAXBUF];

 /* Build the HTTP response body */
 sprintf(body, "<html><title>Tiny Error</title>");
 sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
 sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
 sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
 sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

 /* Print the HTTP response */
 sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
 Rio_writen(fd, buf, strlen(buf));
 sprintf(buf, "Content-type: text/html\r\n");
 Rio_writen(fd, buf, strlen(buf));
 sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
 Rio_writen(fd, buf, strlen(buf));
 Rio_writen(fd, body, strlen(body));

}


//http://beej.us/guide/bgnet/output/html/multipage/sockaddr_inman.html
