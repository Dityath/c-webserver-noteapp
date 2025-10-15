#define _POSIX_C_SOURCE 200809L
#include "repositories.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

char *repository_create_note(MYSQL *conn, char *title, char *content) {
  MYSQL_STMT *stmt;
  MYSQL_BIND bind[2];

  const char *query = "INSERT INTO notes (title, content) VALUES (?, ?)";

  stmt = mysql_stmt_init(conn);
  if (!stmt) {
    fprintf(stderr, "mysql_stmt_init() failed\n");
    return NULL;
  }

  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    fprintf(stderr, "mysql_stmt_prepare() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  memset(bind, 0, sizeof(bind));

  unsigned long title_length = strlen(title);
  unsigned long content_length = strlen(content);

  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)title;
  bind[0].buffer_length = title_length;
  bind[0].length = &title_length;

  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer = (char *)content;
  bind[1].buffer_length = content_length;
  bind[1].length = &content_length;

  if (mysql_stmt_bind_param(stmt, bind)) {
    fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  if (mysql_stmt_execute(stmt)) {
    fprintf(stderr, "mysql_stmt_execute() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  mysql_stmt_close(stmt);
  return "{\"message\": \"Note created successfully\"}";
}

Note *repository_get_note_by_id(MYSQL *conn, int id) {
  MYSQL_STMT *stmt;
  MYSQL_BIND bind[1];
  MYSQL_BIND result_bind[4];

  const char *query =
      "SELECT id, title, content, created_at FROM notes WHERE id = ?";

  stmt = mysql_stmt_init(conn);
  if (!stmt) {
    fprintf(stderr, "mysql_stmt_init() failed\n");
    return NULL;
  }

  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    fprintf(stderr, "mysql_stmt_prepare() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_LONG;
  bind[0].buffer = (char *)&id;
  bind[0].is_null = 0;
  bind[0].length = 0;

  if (mysql_stmt_bind_param(stmt, bind)) {
    fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  if (mysql_stmt_execute(stmt)) {
    fprintf(stderr, "mysql_stmt_execute() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  int result_id;
  char title_buffer[256];
  char content_buffer[1024];
  char created_at_buffer[64];
  unsigned long title_length, content_length, created_at_length;
  my_bool title_is_null, content_is_null, created_at_is_null;

  memset(result_bind, 0, sizeof(result_bind));

  result_bind[0].buffer_type = MYSQL_TYPE_LONG;
  result_bind[0].buffer = (char *)&result_id;

  result_bind[1].buffer_type = MYSQL_TYPE_STRING;
  result_bind[1].buffer = title_buffer;
  result_bind[1].buffer_length = sizeof(title_buffer);
  result_bind[1].length = &title_length;
  result_bind[1].is_null = &title_is_null;

  result_bind[2].buffer_type = MYSQL_TYPE_STRING;
  result_bind[2].buffer = content_buffer;
  result_bind[2].buffer_length = sizeof(content_buffer);
  result_bind[2].length = &content_length;
  result_bind[2].is_null = &content_is_null;

  result_bind[3].buffer_type = MYSQL_TYPE_STRING;
  result_bind[3].buffer = created_at_buffer;
  result_bind[3].buffer_length = sizeof(created_at_buffer);
  result_bind[3].length = &created_at_length;
  result_bind[3].is_null = &created_at_is_null;

  if (mysql_stmt_bind_result(stmt, result_bind)) {
    fprintf(stderr, "mysql_stmt_bind_result() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  Note *note = NULL;
  if (mysql_stmt_fetch(stmt) == 0) {
    note = malloc(sizeof(Note));
    if (note) {
      note->id = result_id;
      note->title = strndup(title_buffer, title_length);
      note->content = strndup(content_buffer, content_length);
      note->created_at = strndup(created_at_buffer, created_at_length);

      if (!note->title || !note->content || !note->created_at) {
        free_note(note);
        note = NULL;
      }
    }
  }

  mysql_stmt_close(stmt);
  return note;
}

char *repository_get_all_notes(MYSQL *conn) {
  if (mysql_query(conn, "SELECT id, title, content, created_at FROM notes")) {
    fprintf(stderr, "SELECT failed: %s\n", mysql_error(conn));
    return NULL;
  }

  MYSQL_RES *result = mysql_store_result(conn);
  if (result == NULL) {
    fprintf(stderr, "mysql_store_result failed: %s\n", mysql_error(conn));
    return NULL;
  }

  size_t buffer_size = BUFFER_SIZE;
  char *json_result = malloc(buffer_size);
  strcpy(json_result, "[");

  MYSQL_ROW row;
  int first = 1;
  while ((row = mysql_fetch_row(result))) {
    if (!first) {
      strcat(json_result, ",");
    }
    char note_json[512];
    sprintf(note_json,
            "{\"id\":%s, \"title\":\"%s\", \"content\":\"%s\", "
            "\"created_at\":\"%s\"}",
            row[0], row[1], row[2], row[3]);

    if (strlen(json_result) + strlen(note_json) + 2 > buffer_size) {
      buffer_size *= 2;
      json_result = realloc(json_result, buffer_size);
    }
    strcat(json_result, note_json);
    first = 0;
  }

  strcat(json_result, "]");
  mysql_free_result(result);
  return json_result;
}

char *repository_update_note_by_id(MYSQL *conn, int id, char *title,
                                   char *content) {
  MYSQL_STMT *stmt;
  MYSQL_BIND bind[3];

  const char *query = "UPDATE notes SET title = ?, content = ? WHERE id = ?";

  stmt = mysql_stmt_init(conn);
  if (!stmt) {
    fprintf(stderr, "mysql_stmt_init() failed\n");
    return NULL;
  }

  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    fprintf(stderr, "mysql_stmt_prepare() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  memset(bind, 0, sizeof(bind));

  unsigned long title_length = strlen(title);
  unsigned long content_length = strlen(content);

  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)title;
  bind[0].buffer_length = title_length;
  bind[0].length = &title_length;

  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer = (char *)content;
  bind[1].buffer_length = content_length;
  bind[1].length = &content_length;

  bind[2].buffer_type = MYSQL_TYPE_LONG;
  bind[2].buffer = (char *)&id;
  bind[2].is_null = 0;
  bind[2].length = 0;

  if (mysql_stmt_bind_param(stmt, bind)) {
    fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  if (mysql_stmt_execute(stmt)) {
    fprintf(stderr, "mysql_stmt_execute() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  mysql_stmt_close(stmt);
  return "{\"message\": \"Note updated successfully\"}";
}

char *repository_delete_note_by_id(MYSQL *conn, int id) {
  MYSQL_STMT *stmt;
  MYSQL_BIND bind[1];

  const char *query = "DELETE FROM notes WHERE id = ?";

  stmt = mysql_stmt_init(conn);
  if (!stmt) {
    fprintf(stderr, "mysql_stmt_init() failed\n");
    return NULL;
  }

  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    fprintf(stderr, "mysql_stmt_prepare() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_LONG;
  bind[0].buffer = (char *)&id;
  bind[0].is_null = 0;
  bind[0].length = 0;

  if (mysql_stmt_bind_param(stmt, bind)) {
    fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  if (mysql_stmt_execute(stmt)) {
    fprintf(stderr, "mysql_stmt_execute() failed: %s\n",
            mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return NULL;
  }

  mysql_stmt_close(stmt);
  return "{\"message\": \"Note deleted successfully\"}";
}

void free_note(Note *note) {
  if (note != NULL) {
    free(note->title);
    free(note->content);
    free(note->created_at);
    free(note);
  }
}
