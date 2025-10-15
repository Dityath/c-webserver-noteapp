#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

#define PORT 8000
#define BUFFER_SIZE 1024

#include "routes.h"
#include "database.h"

#include <time.h>

static volatile sig_atomic_t keep_running = 1;

static void handle_signal(int sig) {
    (void)sig; // Unused parameter
    keep_running = 0;
}

void run_server() {
    MYSQL *conn = db_connect();
    if (conn == NULL) {
        exit(EXIT_FAILURE);
    }

    // Set up signal handlers for graceful shutdown
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        db_disconnect(conn);
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8000
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                                                  &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        db_disconnect(conn);
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8000
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0) {
        perror("bind failed");
        close(server_fd);
        db_disconnect(conn);
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        db_disconnect(conn);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while(keep_running) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t*)&addrlen))<0) {
            if (errno == EINTR) {
                // Interrupted by signal, check if we should continue
                continue;
            }
            perror("accept");
            continue; // Continue accepting other connections instead of exiting
        }

        int buffer_size = BUFFER_SIZE;
        char *buffer = malloc(buffer_size);
        if (!buffer) {
            perror("malloc");
            close(new_socket);
            continue;
        }
        int bytes_read = 0;

        // Read the request into a dynamic buffer
        while (1) {
            int result = read(new_socket, buffer + bytes_read, buffer_size - bytes_read - 1);
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
            // Check if we have received the end of the headers
            if (strstr(buffer, "\r\n\r\n")) {
                char *content_length_str = strstr(buffer, "Content-Length: ");
                if (content_length_str) {
                    int content_length = atoi(content_length_str + 16);
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
            continue; // Skip to next iteration if read failed
        }

        if (bytes_read > 0) {
            time_t now = time(NULL);
            char *time_str = ctime(&now);
            if (time_str) {
                time_str[strlen(time_str) - 1] = '\0';
            }

            // Create a copy for parsing without modifying the original buffer
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
        } else if (bytes_read == 0) {
            printf("Client disconnected.\n");
        }
        free(buffer);
        close(new_socket);
    }

    printf("\nShutting down server gracefully...\n");
    close(server_fd);
    db_disconnect(conn);
    printf("Server shutdown complete.\n");
}
