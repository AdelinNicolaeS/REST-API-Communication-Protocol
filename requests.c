#include "requests.h"
#include "helpers.h"
#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

char *compute_delete_request(char *host, char *url, char *cookies, char *auth) {
  char *message = calloc(BUFLEN, sizeof(char));
  char *line = calloc(LINELEN, sizeof(char));

  sprintf(line, "DELETE %s HTTP/1.1", url);
  compute_message(message, line);

  strcpy(line, "Host: ");
  strcat(line, host);
  compute_message(message, line);

  if (cookies != NULL) {
    strcpy(line, "Cookie: ");
    strcat(line, cookies);
    compute_message(message, line);
  }

  if (auth != NULL) {
    strcpy(line, "Authorization: Bearer ");
    strcat(line, auth);
    compute_message(message, line);
  }

  compute_message(message, "");

  memset(line, 0, LINELEN);
  free(line);
  return message;
}

char *compute_get_request(char *host, char *url, char *cookies, char *auth) {
  char *message = calloc(BUFLEN, sizeof(char));
  char *line = calloc(LINELEN, sizeof(char));

  sprintf(line, "GET %s HTTP/1.1", url);
  compute_message(message, line);

  strcpy(line, "Host: ");
  strcat(line, host);
  compute_message(message, line);

  if (cookies != NULL) {
    strcpy(line, "Cookie: ");
    strcat(line, cookies);
    compute_message(message, line);
  }

  if (auth != NULL) {
    strcpy(line, "Authorization: Bearer ");
    strcat(line, auth);
    compute_message(message, line);
  }

  compute_message(message, "");

  memset(line, 0, LINELEN);
  free(line);
  return message;
}

char *compute_post_request(char *host, char *url, char *content_type,
                           char *body_data, /* 1 */ char *cookies,
                           /*1*/ char *auth) {
  char *message = calloc(BUFLEN, sizeof(char));
  char *line = calloc(LINELEN, sizeof(char));

  sprintf(line, "POST %s HTTP/1.1", url);
  compute_message(message, line);

  strcpy(line, "Host: ");
  strcat(line, host);
  compute_message(message, line);

  strcpy(line, "Content-Type: ");
  strcat(line, content_type);
  compute_message(message, line);

  sprintf(line, "Content-Length: %ld", strlen(body_data));
  compute_message(message, line);

  if (cookies != NULL) {
    strcpy(line, "Cookie: ");
    strcat(line, cookies);
    compute_message(message, line);
  }

  if (auth != NULL) {
    strcpy(line, "Authorization: Bearer ");
    strcat(line, auth);
    compute_message(message, line);
  }

  compute_message(message, "");
  compute_message(message, body_data);

  memset(line, 0, LINELEN);
  free(line);
  return message;
}
