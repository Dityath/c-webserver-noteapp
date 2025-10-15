#ifndef HANDLERS_H
#define HANDLERS_H

#include <mysql/mysql.h>

void handle_api_health(int);
void handle_api_hello(int);
void handle_not_found(int);
void handle_create_note(int, MYSQL *, char *);
void handle_get_note(int, MYSQL *, int);
void handle_get_all_notes(int, MYSQL *);
void handle_update_note(int, MYSQL *, int, char *);
void handle_delete_note(int, MYSQL *, int);

#endif
