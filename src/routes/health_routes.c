#include "health_routes.h"
#include "../handlers/health_handler.h"

static void route_api_health(int client_socket, MYSQL *conn, char *request,
                             int *params) {
  (void)conn;
  (void)request;
  (void)params;
  handle_api_health(client_socket);
}

static void route_api_hello(int client_socket, MYSQL *conn, char *request,
                            int *params) {
  (void)conn;
  (void)request;
  (void)params;
  handle_api_hello(client_socket);
}

void register_health_routes(Route *routes, int *index) {
  routes[(*index)++] = (Route){"GET", METHOD_GET, "/api/health", route_api_health};
  routes[(*index)++] = (Route){"GET", METHOD_GET, "/api", route_api_hello};
}
