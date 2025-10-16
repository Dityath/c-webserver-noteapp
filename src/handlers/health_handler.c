#include "health_handler.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

void handle_api_health(int client_socket) {
  char *body = "{\"status\": \"ok\"}";
  char response[BUFFER_SIZE];
  snprintf(response, sizeof(response),
           "HTTP/1.1 200 OK\r\nContent-Type: "
           "application/json\r\nContent-Length: %ld\r\n\r\n%s",
           strlen(body), body);
  write(client_socket, response, strlen(response));
}

void handle_api_hello(int client_socket) {
  char *body = "{\"message\": \"Hello, World!\"}";
  char response[BUFFER_SIZE];
  snprintf(response, sizeof(response),
           "HTTP/1.1 200 OK\r\nContent-Type: "
           "application/json\r\nContent-Length: %ld\r\n\r\n%s",
           strlen(body), body);
  write(client_socket, response, strlen(response));
}
