#define _POSIX_C_SOURCE 200809L
#include "routes.h"
#include "handlers.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void route(int client_socket, MYSQL *conn, char *request) {
    // Make a copy for parsing since strtok modifies the string
    char *request_copy = strdup(request);
    if (!request_copy) {
        handle_not_found(client_socket);
        return;
    }

    char *method = strtok(request_copy, " ");
    char *uri = strtok(NULL, " ");
    int id;

    if (!method || !uri) {
        free(request_copy);
        handle_not_found(client_socket);
        return;
    }

    if (strcmp(uri, "/api/health") == 0) {
        handle_api_health(client_socket);
    } else if (strcmp(uri, "/api") == 0) {
        handle_api_hello(client_socket);
    } else if (strcmp(uri, "/api/notes") == 0) {
        if (strcmp(method, "POST") == 0) {
            handle_create_note(client_socket, conn, request);
        } else if (strcmp(method, "GET") == 0) {
            handle_get_all_notes(client_socket, conn);
        }
    } else if (sscanf(uri, "/api/notes/%d", &id) == 1) {
        if (strcmp(method, "GET") == 0) {
            handle_get_note(client_socket, conn, id);
        } else if (strcmp(method, "PUT") == 0) {
            handle_update_note(client_socket, conn, id, request);
        } else if (strcmp(method, "DELETE") == 0) {
            handle_delete_note(client_socket, conn, id);
        }
    } else {
        handle_not_found(client_socket);
    }

    free(request_copy);
}
