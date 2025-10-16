#include "notes_routes.h"
#include "../handlers/notes_handler.h"

static void route_notes_list(int client_socket, MYSQL *conn, char *request,
                             int *params) {
  (void)request;
  (void)params;
  handle_get_all_notes(client_socket, conn);
}

static void route_notes_create(int client_socket, MYSQL *conn, char *request,
                               int *params) {
  (void)params;
  handle_create_note(client_socket, conn, request);
}

static void route_notes_get(int client_socket, MYSQL *conn, char *request,
                            int *params) {
  (void)request;
  handle_get_note(client_socket, conn, params[0]);
}

static void route_notes_update(int client_socket, MYSQL *conn, char *request,
                               int *params) {
  handle_update_note(client_socket, conn, params[0], request);
}

static void route_notes_delete(int client_socket, MYSQL *conn, char *request,
                               int *params) {
  (void)request;
  handle_delete_note(client_socket, conn, params[0]);
}

void register_notes_routes(Route *routes, int *index) {
  routes[(*index)++] = (Route){"GET", METHOD_GET, "/api/notes", route_notes_list};
  routes[(*index)++] = (Route){"POST", METHOD_POST, "/api/notes", route_notes_create};
  routes[(*index)++] = (Route){"GET", METHOD_GET, "/api/notes/:id", route_notes_get};
  routes[(*index)++] = (Route){"PUT", METHOD_PUT, "/api/notes/:id", route_notes_update};
  routes[(*index)++] = (Route){"DELETE", METHOD_DELETE, "/api/notes/:id", route_notes_delete};
}
