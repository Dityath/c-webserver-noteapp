#include "routes.h"
#include "handlers.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void route(int client_socket, MYSQL *conn, char *path) {
    char *method = strtok(path, " ");
    char *uri = strtok(NULL, " ");
    int id;

    if (strcmp(uri, "/api/health") == 0) {
        handle_api_health(client_socket);
    } else if (strcmp(uri, "/api") == 0) {
        handle_api_hello(client_socket);
    } else if (strcmp(uri, "/api/notes") == 0) {
        if (strcmp(method, "POST") == 0) {
            handle_create_note(client_socket, conn, path);
        } else if (strcmp(method, "GET") == 0) {
            handle_get_all_notes(client_socket, conn);
        }
    } else if (sscanf(uri, "/api/notes/%d", &id) == 1) {
        if (strcmp(method, "GET") == 0) {
            handle_get_note(client_socket, conn, id);
        } else if (strcmp(method, "PUT") == 0) {
            handle_update_note(client_socket, conn, id, path);
        } else if (strcmp(method, "DELETE") == 0) {
            handle_delete_note(client_socket, conn, id);
        }
    } else {
        handle_not_found(client_socket);
    }
}
