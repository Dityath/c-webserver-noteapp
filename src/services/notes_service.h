#ifndef NOTES_SERVICE_H
#define NOTES_SERVICE_H

#include "../repositories/notes_repository.h"

char *service_create_note(MYSQL *, char *, char *);
Note *service_get_note_by_id(MYSQL *, int);
char *service_get_all_notes(MYSQL *);
char *service_update_note_by_id(MYSQL *, int, char *, char *);
char *service_delete_note_by_id(MYSQL *, int);

#endif
