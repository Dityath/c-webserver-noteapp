#include "notes_handler.h"
#include "../services/notes_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MAX_TITLE_LEN 255
#define MAX_CONTENT_LEN 4095

static int extract_json_string(const char *json, const char *key, char *output,
                               size_t max_len) {
  char search_key[64];
  snprintf(search_key, sizeof(search_key), "\"%s\":\"", key);

  const char *start = strstr(json, search_key);
  if (!start) {
    return 0;
  }

  start += strlen(search_key);
  const char *end = start;
  size_t out_pos = 0;

  while (*end && *end != '\"' && out_pos < max_len - 1) {
    if (*end == '\\' && *(end + 1)) {
      end++; // Skip backslash
      switch (*end) {
      case '\"':
        output[out_pos++] = '\"';
        break;
      case '\\':
        output[out_pos++] = '\\';
        break;
      case '/':
        output[out_pos++] = '/';
        break;
      case 'b':
        output[out_pos++] = '\b';
        break;
      case 'f':
        output[out_pos++] = '\f';
        break;
      case 'n':
        output[out_pos++] = '\n';
        break;
      case 'r':
        output[out_pos++] = '\r';
        break;
      case 't':
        output[out_pos++] = '\t';
        break;
      default:
        output[out_pos++] = *end;
        break;
      }
      end++;
    } else {
      output[out_pos++] = *end++;
    }
  }

  output[out_pos] = '\0';
  return (*end == '\"') ? 1 : 0;
}

static int validate_note_input(const char *title, const char *content) {
  if (!title || !content)
    return 0;

  size_t title_len = strlen(title);
  size_t content_len = strlen(content);

  if (title_len == 0 || title_len > MAX_TITLE_LEN)
    return 0;
  if (content_len == 0 || content_len > MAX_CONTENT_LEN)
    return 0;

  return 1;
}

void handle_not_found(int client_socket) {
  char *body = "{\"error\": \"Not Found\"}";
  char response[BUFFER_SIZE];
  snprintf(response, sizeof(response),
           "HTTP/1.1 404 Not Found\r\nContent-Type: "
           "application/json\r\nContent-Length: %ld\r\n\r\n%s",
           strlen(body), body);
  write(client_socket, response, strlen(response));
}

void handle_create_note(int client_socket, MYSQL *conn, char *buffer) {
  char *body = strstr(buffer, "\r\n\r\n");
  if (body == NULL) {
    char *response =
        "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\": \"Invalid request\"}";
    write(client_socket, response, strlen(response));
    return;
  }

  body += 4; // Skip \r\n\r\n

  char title[MAX_TITLE_LEN + 1] = {0};
  char content[MAX_CONTENT_LEN + 1] = {0};

  if (!extract_json_string(body, "title", title, sizeof(title)) ||
      !extract_json_string(body, "content", content, sizeof(content))) {
    char *response =
        "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\": \"Invalid JSON\"}";
    write(client_socket, response, strlen(response));
    return;
  }

  if (!validate_note_input(title, content)) {
    char *response = "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\": \"Invalid "
                     "title or content\"}";
    write(client_socket, response, strlen(response));
    return;
  }

  char *result = service_create_note(conn, title, content);
  if (result) {
    size_t response_len = strlen(result) + 128;
    char *response = malloc(response_len);
    if (response) {
      snprintf(response, response_len,
               "HTTP/1.1 201 Created\r\nContent-Type: "
               "application/json\r\nContent-Length: %ld\r\n\r\n%s",
               strlen(result), result);
      write(client_socket, response, strlen(response));
      free(response);
    }
  } else {
    char *response = "HTTP/1.1 500 Internal Server Error\r\n\r\n{\"error\": "
                     "\"Database error\"}";
    write(client_socket, response, strlen(response));
  }
}

void handle_get_note(int client_socket, MYSQL *conn, int id) {
  Note *note = service_get_note_by_id(conn, id);
  if (note) {
    char body[BUFFER_SIZE];
    snprintf(body, sizeof(body),
             "{\"id\":%d, \"title\":\"%s\", \"content\":\"%s\", "
             "\"created_at\":\"%s\"}",
             note->id, note->title, note->content, note->created_at);
    size_t response_len = strlen(body) + 128;
    char *response = malloc(response_len);
    snprintf(response, response_len,
             "HTTP/1.1 200 OK\r\nContent-Type: "
             "application/json\r\nContent-Length: %ld\r\n\r\n%s",
             strlen(body), body);
    write(client_socket, response, strlen(response));
    free(response);
    free_note(note);
  } else {
    char *body = "{\"error\": \"Not Found\"}";
    size_t response_len = strlen(body) + 128;
    char *response = malloc(response_len);
    snprintf(response, response_len,
             "HTTP/1.1 404 Not Found\r\nContent-Type: "
             "application/json\r\nContent-Length: %ld\r\n\r\n%s",
             strlen(body), body);
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
    snprintf(response, response_len,
             "HTTP/1.1 200 OK\r\nContent-Type: "
             "application/json\r\nContent-Length: %ld\r\n\r\n%s",
             strlen(body), body);
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
  if (body == NULL) {
    char *response =
        "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\": \"Invalid request\"}";
    write(client_socket, response, strlen(response));
    return;
  }

  body += 4;

  char title[MAX_TITLE_LEN + 1] = {0};
  char content[MAX_CONTENT_LEN + 1] = {0};

  if (!extract_json_string(body, "title", title, sizeof(title)) ||
      !extract_json_string(body, "content", content, sizeof(content))) {
    char *response =
        "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\": \"Invalid JSON\"}";
    write(client_socket, response, strlen(response));
    return;
  }

  if (!validate_note_input(title, content)) {
    char *response = "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\": \"Invalid "
                     "title or content\"}";
    write(client_socket, response, strlen(response));
    return;
  }

  char *result = service_update_note_by_id(conn, id, title, content);
  if (result) {
    size_t response_len = strlen(result) + 128;
    char *response = malloc(response_len);
    if (response) {
      snprintf(response, response_len,
               "HTTP/1.1 200 OK\r\nContent-Type: "
               "application/json\r\nContent-Length: %ld\r\n\r\n%s",
               strlen(result), result);
      write(client_socket, response, strlen(response));
      free(response);
    }
  } else {
    char *response = "HTTP/1.1 500 Internal Server Error\r\n\r\n{\"error\": "
                     "\"Database error\"}";
    write(client_socket, response, strlen(response));
  }
}

void handle_delete_note(int client_socket, MYSQL *conn, int id) {
  char *result = service_delete_note_by_id(conn, id);
  if (result) {
    size_t response_len = strlen(result) + 128;
    char *response = malloc(response_len);
    snprintf(response, response_len,
             "HTTP/1.1 200 OK\r\nContent-Type: "
             "application/json\r\nContent-Length: %ld\r\n\r\n%s",
             strlen(result), result);
    write(client_socket, response, strlen(response));
    free(response);
  } else {
    char *response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
    write(client_socket, response, strlen(response));
  }
}
