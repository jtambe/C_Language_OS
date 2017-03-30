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
 */

#include "csapp.h"

/*
 * Function prototypes
 */

int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void doit( int fd, struct sockaddr_in sockaddr);
void read_header_request(rio_t *rp, char *data, int *length, int *chunked);
int parse_header(char *block_header);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg );
void *thread(void *vargp);
int open_clientfd_ts(char *hostname, int port);

FILE *proxy_log; /* log file*/
static sem_t client_sock_mutex;  /* mutex*/
static sem_t proxy_log_mutex; /* mutex*/

/** Robust I/O routines wrappers*/

void Rio_writen_w(int fd, void *usrbuf, size_t n){
    if (rio_writen(fd, usrbuf, n) != n)
        fprintf(stderr, "Rio_writen_w: %s\n", strerror(errno));
}

ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n){
    ssize_t err;
    if ((err = rio_readnb(rp, usrbuf, n)) < 0) {
        fprintf(stderr, "Rio_readnb_w error: %s\n", strerror(errno));
        return 0;
      }
    return err;
}

ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen){
    ssize_t err;
    if ((err = rio_readlineb(rp, usrbuf, maxlen)) < 0) {
        fprintf(stderr, "Rio_readlineb_w error: %s\n", strerror(errno));
        return 0;
    }
    return err;
}

struct operation{
  int fd;
  struct sockaddr_in sockaddr;
};

/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv)
{
    int listenfd, connfd ,port;
    socklen_t clientlen;
    pthread_t tid;
    struct sockaddr_in clientaddr;

    /* Check arguments */
    if (argc != 2) {
	     fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	      exit(0);
    }

    port = atoi(argv[1]);
    Sem_init(&client_sock_mutex,0,1);
    Sem_init(&proxy_log_mutex,0,1);
    Signal(SIGPIPE, SIG_IGN);

    //open log file in append mode
    proxy_log = fopen("proxy.log","a");

    // open client socket for listining
    listenfd = Open_listenfd(port);
    printf("Proxy server is listening....\n");
    while(1){
      clientlen = sizeof(clientaddr);
      connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
      struct operation *vargp = (struct operation *) Malloc(sizeof(struct operation));
      vargp->fd = connfd;  // set file descriptor to struct pointer fd
      vargp -> sockaddr = clientaddr;
      //create thread
      Pthread_create(&tid, NULL, thread, vargp);//thread is the start_routine()
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
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d", time_str, a, b, c, d, uri, size);
}

/* --------------------------------------------------------------------
Thread routine;
The issue is avoiding memory leaks in the thread routine. Since we are
not explicitly reaping threads, we must detach each thread so that its memory
resources will be reclaimed when it terminates.
--------------------------------------------------------------------*/

void *thread(void *vargp)
{
	Pthread_detach(pthread_self());
	struct operation *thread_opr = (struct operation *) vargp;
	doit(thread_opr->fd, thread_opr->sockaddr);
	close(thread_opr->fd);
  Free(vargp);
	return NULL;
}

/*---------------------------------------------------------------------
HTTP transaction Handler
---------------------------------------------------------------------*/

void doit(int fd, struct sockaddr_in sockaddr){

  rio_t rio_client_fd, rio_server_fd;
  int serverfd, port, data_length, block_encode, block_length, size = 0;
  char hostname[MAXLINE], pathname[MAXLINE], buf[MAXLINE];
  char uri[MAXLINE], version[MAXLINE], logstring[MAXLINE], method[MAXLINE];
  char headers[MAXBUF], request[MAXBUF], response[MAXBUF];
  //The rio_readinitb function sets up
  //an empty read buffer and associates an open file descriptor with that buffer
  Rio_readinitb(&rio_client_fd, fd);
  //copies the minimum of n and rp->rio_cnt bytes from the
  //read buffer to the user buffer and returns the number of bytes copied if its is <=0 thren return
  if (Rio_readlineb_w(&rio_client_fd, buf, MAXLINE) <= 0)
    return;

    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcmp(method, "GET")) {
        clienterror(fd, uri, "501", "Proxy Error", "Proxy doesn't implement this method");
        return;
    }

    read_header_request(&rio_client_fd, headers, &data_length, &block_encode);

    if (parse_uri(uri, hostname, pathname, &port) == -1) {
        clienterror(fd, uri, "404", "Not Found", "Proxy couldn't find this uri");
        return;
    }
    sprintf(request, "%s /%s %s\r\n%s", method, pathname, version, headers);
    //connects
    if ((serverfd = open_clientfd_ts(hostname, port)) == -1)
      return;
    Rio_writen_w(serverfd, request, strlen(request));

    if (strcmp(method, "POST") == 0) {
      Rio_readnb_w(&rio_client_fd, buf, data_length);
      Rio_writen_w(serverfd, buf, data_length);
    }

    Rio_readinitb(&rio_server_fd, serverfd);
    read_header_request(&rio_server_fd, response, &data_length, &block_encode);
    Rio_writen_w(fd, response, strlen(response));

   if (block_encode) {
    	Rio_readlineb_w(&rio_server_fd, buf, MAXLINE);
    	Rio_writen_w(fd, buf, strlen(buf));
    	while ((block_length = parse_header(buf)) > 0) {
            size += block_length;
    		        Rio_readnb_w(&rio_server_fd, buf, block_length);
    		        Rio_writen_w(fd, buf, block_length);
    		Rio_readlineb_w(&rio_server_fd, buf, MAXLINE);
    		Rio_writen_w(fd, buf, strlen(buf));
    		        Rio_readlineb_w(&rio_server_fd, buf, MAXLINE);
    		        Rio_writen_w(fd, buf, strlen(buf));
    	}
    	Rio_readlineb_w(&rio_server_fd, buf, MAXLINE);
    	Rio_writen_w(fd, buf, strlen(buf));
    } else if (data_length > 0) {
         size += data_length;
         int temp_length = data_length;
         int curr_length = 0;
         while (temp_length > 0) {
             curr_length = temp_length > MAXBUF ? MAXBUF : temp_length;
             temp_length -= curr_length;
              Rio_readnb_w(&rio_server_fd, buf, curr_length);
              Rio_writen_w(fd, buf, curr_length);
         }
     } else {
    	while ((block_length = Rio_readlineb_w(&rio_server_fd, buf, MAXBUF)) > 0) {
            size += block_length;
    		Rio_writen_w(fd, buf, block_length);
        }
    }

    //write to proxy.log file
    P(&proxy_log_mutex);
    format_log_entry(logstring, &sockaddr, uri, size);
    fprintf(proxy_log, "%s\n", logstring);
    printf("%s\n", logstring);
    V(&proxy_log_mutex);

    close(serverfd);

}


/*---------------------------------------------------------------------
The read_header_request function
It simply reads and ignores them by calling the read_header_request function.
Chunked transfer encoding is a data transfer mechanism in version 1.1 of the
Hypertext Transfer Protocol (HTTP) in which data is sent in a series of "chunks".
It uses the Transfer-Encoding HTTP header in place of the Content-Length header,
which the earlier version of the protocol would otherwise require.
---------------------------------------------------------------------*/
void read_header_request(rio_t *rp, char *data, int *length, int *chunked)
{
  *length = *chunked = 0;
   char buf[MAXLINE];
   Rio_readlineb_w(rp, buf, MAXLINE);
   strcpy(data, buf);
   strcat(data, "Connection: close\r\n");
   while (strcmp(buf, "\r\n")) { //if not empty
       Rio_readlineb_w(rp, buf, MAXLINE);
      if (strncasecmp(buf, "Content-Length:", 15) == 0)
             *length = atoi(buf + 15);
      if (strncasecmp(buf, "Transfer-Encoding: chunked", 26) == 0)
               *chunked = 1;
         //if header contains Proxy-Connection: or connection field then we have to keep connection alive
       if (strncasecmp(buf, "Proxy-Connection:", 17) == 0 || strncasecmp(buf, "Connection:", 11) == 0)
           continue;
       strcat(data, buf);
   }
}
// returns block lenght to calling function for encoded-chunked header
int parse_header(char *block_header)
{
	char ch;
	int i, length = 0;
	for (i = 0; (ch = block_header[i]) != '\r'; i++)
		if (isdigit(ch))
			length = length*16 + ch - '0';
		else if (ch >= 'A' && ch <= 'F')
			length = length*16 + ch - 'A' + 10;
        else if (ch >= 'a' && ch <= 'f')
          length = length*16 + ch - 'a' + 10;
		else
			return -1;
	return length;
}



/*-------------------------------------------------------------------
open_clientfd_ts: helper function that establishes a connection with
a server.network byte order) to the server’s socket
// REF CSAPP Page no: "904" This function is thread safe
---------------------------------------------------------------------*/
int open_clientfd_ts(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp, *copyp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;

    P(&client_sock_mutex);
    if ((copyp = gethostbyname(hostname)) == NULL) {
    	 V(&client_sock_mutex);
    	  return -2;
    }

    hp = (struct hostent*) malloc(sizeof(struct hostent));
    memcpy(hp, copyp, sizeof(struct hostent));
    V(&client_sock_mutex);
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0],
      (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);
    free(hp);

    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
    return -1;
    return clientfd;
}




/*---------------------------------------------------------------------
The clienterror Function Ref: CSAPP PAGE NO:922
---------------------------------------------------------------------*/

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
