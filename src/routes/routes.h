#ifndef ROUTES_H
#define ROUTES_H

#include <mysql/mysql.h>

typedef void (*RouteHandler)(int client_socket, MYSQL *conn, char *request,
                             int *path_params);

typedef enum {
  METHOD_GET,
  METHOD_POST,
  METHOD_PUT,
  METHOD_DELETE,
  METHOD_UNKNOWN
} HttpMethod;

typedef struct {
  const char *method_str;
  HttpMethod method;
  const char *path_pattern;
  RouteHandler handler;
} Route;

void route(int client_socket, MYSQL *conn, char *request);

#endif
