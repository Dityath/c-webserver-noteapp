#ifndef NOTES_REPOSITORY_H
#define NOTES_REPOSITORY_H

#include <mysql/mysql.h>

typedef struct {
  int id;
  char *title;
  char *content;
  char *created_at;
} Note;

char *repository_create_note(MYSQL *, char *, char *);
Note *repository_get_note_by_id(MYSQL *, int);
char *repository_get_all_notes(MYSQL *);
char *repository_update_note_by_id(MYSQL *, int, char *, char *);
char *repository_delete_note_by_id(MYSQL *, int);
void free_note(Note *);

#endif
