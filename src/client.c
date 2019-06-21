#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "lib.h"

#define BUFSIZE 4096 // max number of bytes we can get at once

/**
 * Struct to hold all three pieces of a URL
 */
typedef struct urlinfo_t {
  char *hostname;
  char *port;
  char *path;
} urlinfo_t;

/**
 * Tokenize the given URL into hostname, path, and port.
 *
 * url: The input URL to parse.
 *
 * Store hostname, path, and port in a urlinfo_t struct and return the struct.
*/
urlinfo_t *parse_url(char *url)
{
  // copy the input URL so as not to mutate the original
  char *hostname = strdup(url);
  char *port;
  char *path;

  urlinfo_t *urlinfo = malloc(sizeof(urlinfo_t));

  /*
    We can parse the input URL by doing the following:

    1. Use strchr to find the first slash in the URL (this is assuming there is no http:// or https:// in the URL).
    2. Set the path pointer to 1 character after the spot returned by strchr.
    3. Overwrite the slash with a '\0' so that we are no longer considering anything after the slash.
    4. Use strchr to find the first colon in the URL.
    5. Set the port pointer to 1 character after the spot returned by strchr.
    6. Overwrite the colon with a '\0' so that we are just left with the hostname.
  */

  ///////////////////
  // IMPLEMENT ME! //
  ///////////////////

  // stripping http:// and https:// off of url if they are present
  char *http = strstr(hostname, "http://");
  char *https = strstr(hostname, "https://");

  if (http != NULL) {
    hostname += 7;
  }

  if (https!= NULL) {
    hostname += 8;
  }

  // setting up pointer to path
  path = strchr(hostname, '/');
  *path = '\0';
  path++;

  // setting up pointer to port and defaulting to port 80 if no colon in url
  port = strchr(hostname, ':');
  if (port != NULL) {
    *port = '\0';
    port++;
  } else {
    port = "80";
  }

  
  // assigning hostname, path and port to the urlinfo struct properties
  urlinfo->hostname = hostname;
  urlinfo->path = path;
  urlinfo->port = port;


  return urlinfo;
}

/**
 * Constructs and sends an HTTP request
 *
 * fd:       The file descriptor of the connection.
 * hostname: The hostname string.
 * port:     The port string.
 * path:     The path string.
 *
 * Return the value from the send() function.
*/
int send_request(int fd, char *hostname, char *port, char *path)
{
  const int max_request_size = 16384;
  char request[max_request_size];
  int rv;

  ///////////////////
  // IMPLEMENT ME! //
  ///////////////////
  int request_length = sprintf(request, "GET /%s HTTP/1.1\n"
                                        "Host: %s:%s\n"
                                        "Connection: close\n"
                                        "\n",
                                        path, hostname, port);
  
  rv = send(fd, request, request_length, 0);

  if (rv < 0) {
    perror("send");
  }

  return rv;
}

int main(int argc, char *argv[])
{  
  int sockfd, numbytes;  
  char buf[BUFSIZE];
  char noheaderbuf[BUFSIZE];

  if (argc < 2 || argc > 3) {
    fprintf(stderr,"usage: client HOSTNAME:PORT/PATH [-h]\n");
    exit(1);
  }

  /*
    1. Parse the input URL
    2. Initialize a socket by calling the `get_socket` function from lib.c
    3. Call `send_request` to construct the request and send it
    4. Call `recv` in a loop until there is no more data to receive from the server. Print the received response to stdout.
    5. Clean up any allocated memory and open file descriptors.
  */

  ///////////////////
  // IMPLEMENT ME! //
  ///////////////////
  urlinfo_t *urlinfo = parse_url(argv[1]);

  sockfd = get_socket(urlinfo->hostname, urlinfo->port);
  send_request(sockfd, urlinfo->hostname, urlinfo->port, urlinfo->path);

  while ((numbytes = recv(sockfd, buf, BUFSIZE - 1, 0)) > 0) {
    // break out of this loop if we get a 301 moved
    if (strstr(buf, "HTTP/1.1 301")) {
      break;
    }
  // print the data we got back to stdout
    if (argc == 3 && strcmp(argv[2], "-h") == 0)
    {
      fprintf(stdout, "%s\n", buf);
    } 
    else if (argc == 2)
    { /* if we don't pass the -h header flag */

      // set up pointer to the double \n which occur at end of header
      char *headless_buf = strstr(buf, "\n\n");
      fprintf(stdout, "%s\n", headless_buf);
    }
  }



  char *redirect = strstr(buf, "HTTP/1.1 301");

  if (redirect != NULL) {
    char *loc = strstr(buf, "http");
    char *endloc = strchr(loc, '\n');
    *endloc = '\0';
    
    urlinfo = parse_url(loc);

    sockfd = get_socket(urlinfo->hostname, urlinfo->port);
    send_request(sockfd, urlinfo->hostname, urlinfo->port, urlinfo->path);

    while ((numbytes = recv(sockfd, buf, BUFSIZE - 1, 0)) > 0) {
    // print the data we got back to stdout
      if (argc == 3 && strcmp(argv[2], "-h") == 0) {
        fprintf(stdout, "%s\n", buf);
      } 
      else if (argc == 2) { /* if we pass the -h no header flag */
        char *newbuf = strstr(buf, "\n\n");
        fprintf(stdout, "%s\n", newbuf);
      }
    }
  }

  free(urlinfo);
  close(sockfd);


  

  return 0;
}
