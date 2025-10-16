#include "server.h"
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8000
#define BUFFER_SIZE 1024
#define MAX_CONTENT_LENGTH (10 * 1024 * 1024)

#include "../routes/routes.h"
#include "database.h"

#include <time.h>

static volatile sig_atomic_t keep_running = 1;

static void handle_signal(int sig) {
  (void)sig;
  keep_running = 0;
}

void run_server() {
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    close(server_fd);
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d\n", PORT);

  while (keep_running) {
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("accept");
      continue;
    }

    int buffer_size = BUFFER_SIZE;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
      perror("malloc");
      close(new_socket);
      continue;
    }
    int bytes_read = 0;

    while (1) {
      int result =
          read(new_socket, buffer + bytes_read, buffer_size - bytes_read - 1);
      if (result < 0) {
        perror("read");
        free(buffer);
        close(new_socket);
        break;
      }
      if (result == 0) {
        break;
      }
      bytes_read += result;
      buffer[bytes_read] = '\0'; // Ensure null termination

      if (bytes_read >= buffer_size - 1) {
        buffer_size *= 2;
        char *new_buffer = realloc(buffer, buffer_size);
        if (!new_buffer) {
          perror("realloc");
          free(buffer);
          close(new_socket);
          break;
        }
        buffer = new_buffer;
      }
      if (strstr(buffer, "\r\n\r\n")) {
        char *content_length_str = strstr(buffer, "Content-Length: ");
        if (content_length_str) {
          int content_length = atoi(content_length_str + 16);
          // Validate Content-Length to prevent DoS
          if (content_length < 0 || content_length > MAX_CONTENT_LENGTH) {
            fprintf(stderr, "Invalid Content-Length: %d\n", content_length);
            free(buffer);
            close(new_socket);
            break;
          }
          char *body = strstr(buffer, "\r\n\r\n");
          if (body) {
            body += 4;
            int header_len = body - buffer;
            if (bytes_read >= header_len + content_length) {
              break;
            }
          }
        } else {
          break;
        }
      }
    }

    if (bytes_read < 0) {
      continue;
    }

    if (bytes_read > 0) {
      MYSQL *conn = db_connect();
      if (conn == NULL) {
        char *response =
            "HTTP/1.1 500 Internal Server Error\r\n\r\n{\"error\": \"Database "
            "connection failed\"}";
        write(new_socket, response, strlen(response));
        free(buffer);
        close(new_socket);
        continue;
      }

      time_t now = time(NULL);
      char *time_str = ctime(&now);
      if (time_str) {
        time_str[strlen(time_str) - 1] = '\0';
      }

      char *buffer_copy = malloc(bytes_read + 1);
      if (buffer_copy) {
        memcpy(buffer_copy, buffer, bytes_read);
        buffer_copy[bytes_read] = '\0';

        char *method = strtok(buffer_copy, " ");
        char *uri = strtok(NULL, " ");

        if (method && uri && time_str) {
          printf("[%s] %s %s\n", time_str, method, uri);
        }

        route(new_socket, conn, buffer);
        free(buffer_copy);
      } else {
        route(new_socket, conn, buffer);
      }

      db_disconnect(conn);
    } else if (bytes_read == 0) {
      printf("Client disconnected.\n");
    }
    free(buffer);
    close(new_socket);
  }

  printf("\nShutting down server gracefully...\n");
  close(server_fd);
  printf("Server shutdown complete.\n");
}
