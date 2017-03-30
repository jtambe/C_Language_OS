/*
 * File: util.c
 * Author: Andy Sayler
 * Project: CSCI 3753 Programming Assignment 2
 * Create Date: 2012/02/01
 * Modify Date: 2012/02/01
 * Description:
 * 	This file contains declarations of utility functions for
 *      Programming Assignment 2.
 *
 */

//http://man7.org/linux/man-pages/man3/getaddrinfo.3.html

#include "util.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
int dnslookup(const char* hostname, char* firstIPstr, int maxSize){

    /* Local vars */
    struct addrinfo* headresult = NULL;
    struct addrinfo* result = NULL;
    struct sockaddr_in* ipv4sock = NULL;
    struct in_addr* ipv4addr = NULL;

    //code for IPv6
    struct addrinfo hints;
/*
If hints.ai_flags specifies the AI_V4MAPPED flag, and hints.ai_family
       was specified as AF_INET6, and no matching IPv6 addresses could be
       found, then return IPv4-mapped IPv6 addresses in the list pointed to
       by res.  If both AI_V4MAPPED and AI_ALL are specified in
       hints.ai_flags, then return both IPv6 and IPv4-mapped IPv6 addresses
       in the list pointed to by res.  AI_ALL is ignored if AI_V4MAPPED is
       not also specified.
*/
    hints.ai_family = AF_INET6 | AF_INET;
    // SOCK_STREAM, SOCK_DGRAM
    hints.ai_socktype = 0;
    // AI_V4MAPPED only checks for IPV6
    //hints.ai_flags = AI_V4MAPPED | AI_CANONNAME | AI_ADDRCONFIG | AI_ALL ;;
    hints.ai_flags =  AI_ADDRCONFIG | AI_V4MAPPED ;
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    struct sockaddr_in6* ipv6sock = NULL;
    struct in6_addr* ipv6addr = NULL;

    char ipv4str[INET_ADDRSTRLEN];
    char ipstr[INET6_ADDRSTRLEN];
    int addrError = 0;

    /* DEBUG: Print Hostname*/
#ifdef UTIL_DEBUG
    fprintf(stderr, "%s\n", hostname);
#endif

    /* Lookup Hostname */
    addrError = getaddrinfo(hostname, NULL, &hints, &headresult);
    if(addrError)
    {
      fprintf(stderr, "Error looking up Address: %s\n",
      gai_strerror(addrError));
    	return UTIL_FAILURE;
    }
    /* Loop Through result Linked List */
    for(result=headresult; result != NULL; result = result->ai_next){
	/* Extract IP Address and Convert to String */
	if(result->ai_addr->sa_family == AF_INET6){
	    /* IPv6 Handling */
      //printf("inpv6 flag \n" );
	    ipv6sock = (struct sockaddr_in6*)(result->ai_addr);
	    ipv6addr = &(ipv6sock->sin6_addr);
      // https://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
      // inet_ntop works for both IPv4 and IPv6
      //inet_ntop(AF_INET6, &(sa.sin6_addr), str, INET6_ADDRSTRLEN);
      if(!inet_ntop(AF_INET6, ipv6addr,ipstr, INET6_ADDRSTRLEN))
      //if(!inet_ntop(result->ai_family, ipv6addr,ipstr, sizeof(ipstr)))
      {
		      perror("Error Converting IP to String");
		        return UTIL_FAILURE;
	    }
      //printf("%s \n",ipstr);
      #ifdef UTIL_DEBUG
      	    fprintf(stdout, "%s\n", ipstr);
      #endif
      	    strncpy(ipstr, ipstr, sizeof(ipstr));
      	    ipstr[sizeof(ipstr)-1] = '\0';
        //printf(" IPv6 %s \n",ipstr);
      //
      // #ifdef UTIL_DEBUG
      // 	    fprintf(stdout, "IPv6 Address: Not Handled\n");
      // #endif
      // 	    strncpy(ipstr, "UNHANDELED", sizeof(ipstr));
      // 	    ipstr[sizeof(ipstr)-1] = '\0';
	}
  else if(result->ai_addr->sa_family == AF_INET)
  {
      //printf("entered ipv4 flag \n");
      /* IPv4 Address Handling */
      ipv4sock = (struct sockaddr_in*)(result->ai_addr);
      ipv4addr = &(ipv4sock->sin_addr);
      if(!inet_ntop(result->ai_family, ipv4addr,ipv4str, sizeof(ipv4str)))
      {
        perror("Error Converting IP to String");
        return UTIL_FAILURE;
      }
      #ifdef UTIL_DEBUG
            fprintf(stdout, "%s\n", ipv4str);
      #endif
            strncpy(ipv4str, ipv4str, sizeof(ipv4str));
            ipv4str[sizeof(ipv4str)-1] = '\0';
  }
	else{
	    /* Unhandlded Protocol Handling */
#ifdef UTIL_DEBUG
	    fprintf(stdout, "Unknown Protocol: Not Handled\n");
#endif
	    strncpy(ipstr, "UNHANDELED", sizeof(ipstr));
	    ipstr[sizeof(ipstr)-1] = '\0';
	}
	/* Save First IP Address */
	if(result==headresult){
      //printf("%s \n",ipstr);
      //strncpy(firstIPstr, ipstr, sizeof(ipstr));
      if(result->ai_addr->sa_family == AF_INET6)
      {
          strncpy(firstIPstr, ipstr, maxSize);
      }
      else if(result->ai_addr->sa_family == AF_INET)
      {
        strncpy(firstIPstr, ipv4str, maxSize);
      }

	    firstIPstr[maxSize-1] = '\0';
	}
    }

    /* Cleanup */
    freeaddrinfo(headresult);

    return UTIL_SUCCESS;
}
