#define _POSIX_C_SOURCE 200809L
#include "repositories.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

char* repository_create_note(MYSQL *conn, char *title, char *content) {
    char query[512];
    sprintf(query, "INSERT INTO notes (title, content) VALUES ('%s', '%s')", title, content);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "INSERT failed: %s\n", mysql_error(conn));
        return NULL;
    }

    return "{\"message\": \"Note created successfully\"}";
}

Note* repository_get_note_by_id(MYSQL *conn, int id) {
    char query[512];
    sprintf(query, "SELECT id, title, content, created_at FROM notes WHERE id = %d", id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "SELECT failed: %s\n", mysql_error(conn));
        return NULL;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "mysql_store_result failed: %s\n", mysql_error(conn));
        return NULL;
    }

    MYSQL_ROW row;
    Note *note = NULL;
    if ((row = mysql_fetch_row(result))) {
        note = malloc(sizeof(Note));
        note->id = atoi(row[0]);
        note->title = strdup(row[1]);
        note->content = strdup(row[2]);
        note->created_at = strdup(row[3]);
    }

    mysql_free_result(result);
    return note;
}

char* repository_get_all_notes(MYSQL *conn) {
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
        sprintf(note_json, "{\"id\":%s, \"title\":\"%s\", \"content\":\"%s\", \"created_at\":\"%s\"}", 
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

char* repository_update_note_by_id(MYSQL *conn, int id, char *title, char *content) {
    char query[512];
    sprintf(query, "UPDATE notes SET title = '%s', content = '%s' WHERE id = %d", title, content, id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "UPDATE failed: %s\n", mysql_error(conn));
        return NULL;
    }

    return "{\"message\": \"Note updated successfully\"}";
}

char* repository_delete_note_by_id(MYSQL *conn, int id) {
    char query[512];
    sprintf(query, "DELETE FROM notes WHERE id = %d", id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "DELETE failed: %s\n", mysql_error(conn));
        return NULL;
    }

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
