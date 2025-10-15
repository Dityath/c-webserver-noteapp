#include "services.h"

char *service_create_note(MYSQL *conn, char *title, char *content) {
  return repository_create_note(conn, title, content);
}

Note *service_get_note_by_id(MYSQL *conn, int id) {
  return repository_get_note_by_id(conn, id);
}

char *service_get_all_notes(MYSQL *conn) {
  return repository_get_all_notes(conn);
}

char *service_update_note_by_id(MYSQL *conn, int id, char *title,
                                char *content) {
  return repository_update_note_by_id(conn, id, title, content);
}

char *service_delete_note_by_id(MYSQL *conn, int id) {
  return repository_delete_note_by_id(conn, id);
}
