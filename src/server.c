#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000
#define BUFFER_SIZE 1024

#include "routes.h"
#include "database.h"

#include <time.h>

void run_server() {
    MYSQL *conn = db_connect();
    if (conn == NULL) {
        exit(EXIT_FAILURE);
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8000
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                                                  &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8000
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        int buffer_size = BUFFER_SIZE;
        char *buffer = malloc(buffer_size);
        int bytes_read = 0;

        // Read the request into a dynamic buffer
        while (1) {
            int result = read(new_socket, buffer + bytes_read, buffer_size - bytes_read);
            if (result < 0) {
                perror("read");
                close(new_socket);
                break;
            }
            if (result == 0) {
                break;
            }
            bytes_read += result;
            if (bytes_read == buffer_size) {
                buffer_size *= 2;
                buffer = realloc(buffer, buffer_size);
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

        if (bytes_read > 0) {
            time_t now = time(NULL);
            char *time_str = ctime(&now);
            time_str[strlen(time_str) - 1] = '\0';
            char *buffer_copy = malloc(bytes_read + 1);
            strcpy(buffer_copy, buffer);
            char *method = strtok(buffer_copy, " ");
            char *uri = strtok(NULL, " ");
            printf("[%s] %s %s\n", time_str, method, uri);
            route(new_socket, conn, buffer);
            free(buffer_copy);
        } else if (bytes_read == 0) {
            printf("Client disconnected.\n");
        } else {
            perror("read");
        }
        free(buffer);
        close(new_socket);
    }

    db_disconnect(conn);
}
