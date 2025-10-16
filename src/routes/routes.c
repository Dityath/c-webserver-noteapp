#define _POSIX_C_SOURCE 200809L
#include "routes.h"
#include "../handlers/notes_handler.h"
#include "health_routes.h"
#include "notes_routes.h"
#include <ctype.h>
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

static Route routes[50];
static int routes_initialized = 0;

static void initialize_routes() {
  if (routes_initialized)
    return;

  int index = 0;

  register_health_routes(routes, &index);

  register_notes_routes(routes, &index);

  routes[index] = (Route){NULL, METHOD_UNKNOWN, NULL, NULL};

  routes_initialized = 1;
}

void route(int client_socket, MYSQL *conn, char *request) {
  initialize_routes();

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
