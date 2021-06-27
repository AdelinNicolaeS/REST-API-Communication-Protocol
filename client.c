#include "helpers.h"
#include "parson.h"
#include "requests.h"
#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#define HOST "34.118.48.238"
#define PORT 8080

// check if the answer is OK
int correct_code(char *output) {
  char output_copy[1000];
  strcpy(output_copy, output);
  char *aux = strtok(output_copy, " ");
  aux = strtok(NULL, " ");
  return (strcmp(aux, "200") == 0);
}

void new_register() {
  int sockfd;
  char username[100], password[100];

  // get username and password
  printf("username=");
  getchar();
  fgets(username, 100, stdin);
  username[strlen(username) - 1] = '\0';
  printf("password=");
  fgets(password, 100, stdin);
  password[strlen(password) - 1] = '\0';

  // create the JSON object
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *payload = NULL;
  json_object_set_string(root_object, "username", username);
  json_object_set_string(root_object, "password", password);
  payload = json_serialize_to_string_pretty(root_value);

  // open connection and send the payload
  sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
  char *output;
  output = compute_post_request(HOST, "/api/v1/tema/auth/register",
                                "application/json", payload, NULL, NULL);
  send_to_server(sockfd, output);

  // check if the message is correct
  char *back = receive_from_server(sockfd);
  char *aux = strtok(back, " ");
  aux = strtok(NULL, " ");
  if (strcmp(aux, "201") == 0) {
    printf("Inregistrarea lui %s a avut loc cu succes\n", username);
  } else {
    printf("Acest username este deja folosit\n");
  }

  // free memory
  json_value_free(root_value);
  json_free_serialized_string(payload);
  free(back);
  free(output);
  close(sockfd);
}

void new_login(char *my_cookie, int *logat) {
  int sockfd;
  char username[100], password[100];

  // read username and password
  printf("username=");
  getchar();
  fgets(username, 100, stdin);
  username[strlen(username) - 1] = '\0';
  printf("password=");
  fgets(password, 100, stdin);
  password[strlen(password) - 1] = '\0';

  // build the JSON object
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *payload = NULL;
  json_object_set_string(root_object, "username", username);
  json_object_set_string(root_object, "password", password);
  payload = json_serialize_to_string_pretty(root_value);

  // send the payload to the server
  sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
  char *output;
  output = compute_post_request(HOST, "/api/v1/tema/auth/login",
                                "application/json", payload, NULL, NULL);
  send_to_server(sockfd, output);

  // get the response and keep the cookie
  char *back = receive_from_server(sockfd);
  char *aux = strstr(back, "connect.sid=");
  if (aux == NULL) {
    printf("Credentialele nu se potrivesc\n");
    json_value_free(root_value);
    json_free_serialized_string(payload);
    free(back);
    free(output);
    close(sockfd);
    return;
  }
  printf("Logarea a avut loc cu succes\n");
  *logat = 1;
  char *new_aux = strtok(aux, ";");
  strcpy(my_cookie, new_aux);

  // free memory
  json_value_free(root_value);
  json_free_serialized_string(payload);
  free(back);
  free(output);
  close(sockfd);
}

void enter_library(char *my_cookie, int *logat, char *my_token, int *cu_token) {
  if (*logat == 0) {
    printf("Trebuie sa va logati mai intai\n");
    return;
  }

  // send the message to the server
  int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
  char *output =
      compute_get_request(HOST, "/api/v1/tema/library/access", my_cookie, NULL);
  send_to_server(sockfd, output);

  // keep the token for the access
  char *back = receive_from_server(sockfd);
  if (correct_code(back)) {
    char *word = strstr(back, "{");
    word = word + 10;
    word[strlen(word) - 1] = '\0';
    word[strlen(word) - 1] = '\0';

    *cu_token = 1;
    strcpy(my_token, word);
    printf("Acces permis la biblioteca\n");
  } else {
    printf("Acces in biblioteca refuzat\n");
  }

  free(back);
  free(output);
  close(sockfd);
}

void get_books(char *my_cookie, int *logat, char *my_token, int *cu_token) {
  if (*logat == 0) {
    printf("Trebuie sa va logati mai intai\n");
    return;
  }
  if (*cu_token == 0) {
    printf("Nu ati putut demonstra ca aveti acces la biblioteca\n");
    return;
  }

  // open connection and send message to the server
  int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
  char *output = compute_get_request(HOST, "/api/v1/tema/library/books",
                                     my_cookie, my_token);
  send_to_server(sockfd, output);
  char *back = receive_from_server(sockfd);

  // get the JSON from the answer and print it
  char *books = strstr(back, "[");
  JSON_Value *root_value = json_parse_string(books);
  char *payload = json_serialize_to_string_pretty(root_value);
  printf("%s\n", payload);

  // free memory
  json_value_free(root_value);
  json_free_serialized_string(payload);
  free(output);
  free(back);
  close(sockfd);
}

void add_book(char *my_cookie, int *logat, char *my_token, int *cu_token) {
  // check the access
  if (*logat == 0) {
    printf("Trebuie sa va logati mai intai\n");
    return;
  }
  if (*cu_token == 0) {
    printf("Nu ati putut demonstra ca aveti acces la biblioteca\n");
    return;
  }

  int sockfd;
  char title[100], author[100], genre[100], publisher[100];
  int page_count;

  // read the parameters
  printf("title=");
  getchar();
  fgets(title, 100, stdin);
  title[strlen(title) - 1] = '\0';

  printf("author=");
  fgets(author, 100, stdin);
  author[strlen(author) - 1] = '\0';

  printf("genre=");
  fgets(genre, 100, stdin);
  genre[strlen(genre) - 1] = '\0';

  printf("page_count=");
  int x = scanf("%d", &page_count);
  if (x == 0) {
    printf("Cartea nu s-a putut adauga, datele sunt introduse gresit\n");
    return;
  }

  printf("publisher=");
  getchar();
  fgets(publisher, 100, stdin);
  publisher[strlen(publisher) - 1] = '\0';

  // build the json
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *payload = NULL;
  json_object_set_string(root_object, "title", title);
  json_object_set_string(root_object, "author", author);
  json_object_set_string(root_object, "genre", genre);
  json_object_set_number(root_object, "page_count", page_count);
  json_object_set_string(root_object, "publisher", publisher);
  payload = json_serialize_to_string_pretty(root_value);

  // send it
  sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
  char *output;
  output =
      compute_post_request(HOST, "/api/v1/tema/library/books",
                           "application/json", payload, my_cookie, my_token);
  send_to_server(sockfd, output);

  // check the answer
  char *back = receive_from_server(sockfd);
  if (correct_code(back)) {
    printf("Cartea urmatoare a fost adaugata cu succes:\n%s\n", payload);
  } else {
    printf("Cartea nu s-a putut adauga\n%s\n", back);
  }

  // free memory
  json_value_free(root_value);
  json_free_serialized_string(payload);
  free(back);
  free(output);
  close(sockfd);
}

void get_book(char *my_cookie, int *logat, char *my_token, int *cu_token) {
  // check the access
  if (*logat == 0) {
    printf("Trebuie sa va logati mai intai\n");
    return;
  }
  if (*cu_token == 0) {
    printf("Nu ati putut demonstra ca aveti acces la biblioteca\n");
    return;
  }

  // read id and build the url
  int sockfd, id;
  printf("id=");
  int x = scanf("%d", &id);
  if (x == 0) {
    printf("Id-ul primit nu e valid\n");
    return;
  }
  char url[100];
  sprintf(url, "/api/v1/tema/library/books/%d", id);
  sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
  char *output;

  // compute the request and send it
  output = compute_get_request(HOST, url, my_cookie, my_token);
  send_to_server(sockfd, output);

  // check the response from the server
  char *back = receive_from_server(sockfd);
  if (correct_code(back)) {
    char *book = strstr(back, "{");
    book[strlen(book) - 1] = '\0';
    JSON_Value *root_value = json_parse_string(book);
    char *payload = json_serialize_to_string_pretty(root_value);
    printf("%s\n", payload);

    json_value_free(root_value);
    json_free_serialized_string(payload);
  } else {
    printf("Cartea nu a fost gasita\n");
  }

  free(back);
  free(output);
  close(sockfd);
}

void delete_book(char *my_cookie, int *logat, char *my_token, int *cu_token) {
  if (*logat == 0) {
    printf("Trebuie sa va logati mai intai\n");
    return;
  }
  if (*cu_token == 0) {
    printf("Nu ati putut demonstra ca aveti acces la biblioteca\n");
    return;
  }

  // read id and build the url
  int sockfd, id;
  printf("id=");
  int x = scanf("%d", &id);
  if (x == 0) {
    printf("Id-ul primit nu e valid\n");
    return;
  }
  char url[100];
  sprintf(url, "/api/v1/tema/library/books/%d", id);

  sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
  char *output;
  output = compute_delete_request(HOST, url, my_cookie, my_token);
  send_to_server(sockfd, output);

  char *back = receive_from_server(sockfd);
  // check the answer
  if (correct_code(back)) {
    printf("Cartea cu id-ul %d a fost stearsa cu succes. Cartile ramase sunt "
           "urmatoarele:\n",
           id);
    get_books(my_cookie, logat, my_token, cu_token);
  } else {
    printf("Cartea cu id-ul %d nu a fost gasita\n", id);
  }

  free(back);
  free(output);
  close(sockfd);
}

void logout(char *my_cookie, int *logat) {
  if (*logat == 0) {
    printf("Trebuie sa va logati mai intai\n");
    return;
  }

  // open connection and sene request
  int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
  char *output;
  output =
      compute_get_request(HOST, "/api/v1/tema/auth/logout", my_cookie, NULL);
  send_to_server(sockfd, output);

  // verify the output message
  char *back = receive_from_server(sockfd);
  if (correct_code(back)) {
    *logat = 0;
    printf("Delogarea a avut loc cu succes\n");
  } else {
    printf("Delogarea nu s-a putut realiza\n");
  }

  free(output);
  free(back);
  close(sockfd);
}

int main(int argc, char *argv[]) {
  char *message = malloc(sizeof(char) * 1000);
  char *my_cookie = malloc(sizeof(char) * 1000);
  char *my_token = malloc(sizeof(char) * 1000);
  int logat = 0, cu_token = 0;

  while (1) {
    scanf("%s", message);
    if (strcmp(message, "exit") == 0) {
      break;
    }
    if (strcmp(message, "register") == 0) {
      new_register();
    } else if (strcmp(message, "login") == 0) {
      new_login(my_cookie, &logat);
    } else if (strcmp(message, "enter_library") == 0) {
      enter_library(my_cookie, &logat, my_token, &cu_token);
    } else if (strcmp(message, "get_books") == 0) {
      get_books(my_cookie, &logat, my_token, &cu_token);
    } else if (strcmp(message, "add_book") == 0) {
      add_book(my_cookie, &logat, my_token, &cu_token);
    } else if (strcmp(message, "get_book") == 0) {
      get_book(my_cookie, &logat, my_token, &cu_token);
    } else if (strcmp(message, "delete_book") == 0) {
      delete_book(my_cookie, &logat, my_token, &cu_token);
    } else if (strcmp(message, "logout") == 0) {
      logout(my_cookie, &logat);
    } else {
      printf("Ati introdus comanda gresita!\n");
    }
  }

  free(my_cookie);
  free(message);
  free(my_token);
  return 0;
}
