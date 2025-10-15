#define _POSIX_C_SOURCE 200809L
#include "routes.h"
#include "handlers.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static HttpMethod parse_method(const char *method_str) {
  if (!method_str)
    return METHOD_UNKNOWN;
  if (strcmp(method_str, "GET") == 0)
    return METHOD_GET;
  if (strcmp(method_str, "POST") == 0)
    return METHOD_POST;
  if (strcmp(method_str, "PUT") == 0)
    return METHOD_PUT;
  if (strcmp(method_str, "DELETE") == 0)
    return METHOD_DELETE;
  return METHOD_UNKNOWN;
}

static int match_route(const char *pattern, const char *uri, int *params,
                       int max_params) {
  const char *p = pattern;
  const char *u = uri;
  int param_count = 0;

  while (*p && *u) {
    if (*p == ':') {
      p++;
      while (*p && *p != '/')
        p++;

      int value = 0;
      if (!isdigit(*u))
        return 0;
      while (isdigit(*u)) {
        value = value * 10 + (*u - '0');
        u++;
      }

      if (param_count < max_params) {
        params[param_count++] = value;
      }
    } else if (*p == *u) {
      p++;
      u++;
    } else {
      return 0;
    }
  }

  return (*p == '\0' && *u == '\0');
}

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

static Route routes[] = {
    {"GET", METHOD_GET, "/api/health", route_api_health},
    {"GET", METHOD_GET, "/api", route_api_hello},
    {"GET", METHOD_GET, "/api/notes", route_notes_list},
    {"POST", METHOD_POST, "/api/notes", route_notes_create},
    {"GET", METHOD_GET, "/api/notes/:id", route_notes_get},
    {"PUT", METHOD_PUT, "/api/notes/:id", route_notes_update},
    {"DELETE", METHOD_DELETE, "/api/notes/:id", route_notes_delete},
    {NULL, METHOD_UNKNOWN, NULL, NULL} // Sentinel
};

void route(int client_socket, MYSQL *conn, char *request) {
  // Parse request to extract method and URI
  char *request_copy = strdup(request);
  if (!request_copy) {
    handle_not_found(client_socket);
    return;
  }

  char *method_str = strtok(request_copy, " ");
  char *uri = strtok(NULL, " ");

  if (!method_str || !uri) {
    free(request_copy);
    handle_not_found(client_socket);
    return;
  }

  HttpMethod method = parse_method(method_str);

  int path_params[10] = {0};
  int matched = 0;

  for (int i = 0; routes[i].path_pattern != NULL; i++) {
    if (routes[i].method == method &&
        match_route(routes[i].path_pattern, uri, path_params, 10)) {
      routes[i].handler(client_socket, conn, request, path_params);
      matched = 1;
      break;
    }
  }

  if (!matched) {
    handle_not_found(client_socket);
  }

  free(request_copy);
}
