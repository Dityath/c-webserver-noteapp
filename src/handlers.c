#include "handlers.h"
#include "services.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

void handle_api_health(int client_socket) {
    char *body = "{\"status\": \"ok\"}";
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", strlen(body), body);
    write(client_socket, response, strlen(response));
}

void handle_api_hello(int client_socket) {
    char *body = "{\"message\": \"Hello, World!\"}";
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", strlen(body), body);
    write(client_socket, response, strlen(response));
}

void handle_not_found(int client_socket) {
    char *body = "{\"error\": \"Not Found\"}";
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", strlen(body), body);
    write(client_socket, response, strlen(response));
}

void handle_create_note(int client_socket, MYSQL *conn, char *buffer) {
    char *body = strstr(buffer, "\r\n\r\n");
    if (body != NULL) {
        body += 4; // Skip \r\n\r\n

        char title[256] = {0};
        char content[1024] = {0};

        // A simple JSON parser
        char *title_start = strstr(body, "\"title\":\"");
        if (title_start) {
            title_start += 9;
            char *title_end = strchr(title_start, '\"');
            if (title_end) {
                strncpy(title, title_start, title_end - title_start);
            }
        }
        char *content_start = strstr(body, "\"content\":\"");
        if (content_start) {
            content_start += 11;
            char *content_end = strchr(content_start, '\"');
            if (content_end) {
                strncpy(content, content_start, content_end - content_start);
            }
        }

        if (strlen(title) > 0 && strlen(content) > 0) {
            char *result = service_create_note(conn, title, content);
            if (result) {
                size_t response_len = strlen(result) + 128;
                char *response = malloc(response_len);
                snprintf(response, response_len, "HTTP/1.1 201 Created\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", strlen(result), result);
                write(client_socket, response, strlen(response));
                free(response);
            } else {
                char *response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
                write(client_socket, response, strlen(response));
            }
        } else {
            char *response = "HTTP/1.1 400 Bad Request\r\n\r\n";
            write(client_socket, response, strlen(response));
        }
    }
}

void handle_get_note(int client_socket, MYSQL *conn, int id) {
    Note *note = service_get_note_by_id(conn, id);
    if (note) {
        char body[BUFFER_SIZE];
        snprintf(body, sizeof(body), "{\"id\":%d, \"title\":\"%s\", \"content\":\"%s\", \"created_at\":\"%s\"}", 
            note->id, note->title, note->content, note->created_at);
        size_t response_len = strlen(body) + 128;
        char *response = malloc(response_len);
        snprintf(response, response_len, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", strlen(body), body);
        write(client_socket, response, strlen(response));
        free(response);
        free_note(note);
    } else {
        char *body = "{\"error\": \"Not Found\"}";
        size_t response_len = strlen(body) + 128;
        char *response = malloc(response_len);
        snprintf(response, response_len, "HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", strlen(body), body);
        write(client_socket, response, strlen(response));
        free(response);
    }
}

void handle_get_all_notes(int client_socket, MYSQL *conn) {
    char *result = service_get_all_notes(conn);
    if (result) {
        char body[BUFFER_SIZE];
        snprintf(body, sizeof(body), "{\"data\":%s}", result);
        size_t response_len = strlen(body) + 128;
        char *response = malloc(response_len);
        snprintf(response, response_len, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", strlen(body), body);
        write(client_socket, response, strlen(response));
        free(response);
        free(result);
    } else {
        char *response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        write(client_socket, response, strlen(response));
    }
}

void handle_update_note(int client_socket, MYSQL *conn, int id, char *buffer) {
    char *body = strstr(buffer, "\r\n\r\n");
    if (body != NULL) {
        body += 4; // Skip \r\n\r\n

        char title[256] = {0};
        char content[1024] = {0};

        // A simple JSON parser
        char *title_start = strstr(body, "\"title\":\"");
        if (title_start) {
            title_start += 9;
            char *title_end = strchr(title_start, '\"');
            if (title_end) {
                strncpy(title, title_start, title_end - title_start);
            }
        }
        char *content_start = strstr(body, "\"content\":\"");
        if (content_start) {
            content_start += 11;
            char *content_end = strchr(content_start, '\"');
            if (content_end) {
                strncpy(content, content_start, content_end - content_start);
            }
        }

        if (strlen(title) > 0 && strlen(content) > 0) {
            char *result = service_update_note_by_id(conn, id, title, content);
            if (result) {
                size_t response_len = strlen(result) + 128;
                char *response = malloc(response_len);
                snprintf(response, response_len, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", strlen(result), result);
                write(client_socket, response, strlen(response));
                free(response);
            } else {
                char *response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
                write(client_socket, response, strlen(response));
            }
        } else {
            char *response = "HTTP/1.1 400 Bad Request\r\n\r\n";
            write(client_socket, response, strlen(response));
        }
    }
}

void handle_delete_note(int client_socket, MYSQL *conn, int id) {
    char *result = service_delete_note_by_id(conn, id);
    if (result) {
        size_t response_len = strlen(result) + 128;
        char *response = malloc(response_len);
        snprintf(response, response_len, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s", strlen(result), result);
        write(client_socket, response, strlen(response));
        free(response);
    } else {
        char *response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        write(client_socket, response, strlen(response));
    }
}
