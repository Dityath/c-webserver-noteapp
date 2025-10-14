#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <mysql/mysql.h>
#include "../src/config.h"

int main() {
    Config *config = get_config();
    if (config == NULL) {
        return 1;
    }

    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        free_config(config);
        return 1;
    }

    if (mysql_real_connect(conn, config->host, config->username, config->password, 
            config->database, atoi(config->port), NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        free_config(config);
        return 1;
    }

    struct dirent *de;
    DIR *dr = opendir("db/migrations");

    if (dr == NULL) {
        printf("Could not open current directory" );
        return 0;
    }

    while ((de = readdir(dr)) != NULL) {
        if (strstr(de->d_name, ".sql")) {
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "db/migrations/%s", de->d_name);
            FILE *file = fopen(file_path, "r");
            if (file) {
                char query[4096];
                size_t query_len = fread(query, 1, 4096, file);
                query[query_len] = '\0';
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "Migration failed for %s: %s\n", de->d_name, mysql_error(conn));
                }
                fclose(file);
            }
        }
    }

    closedir(dr);    
    mysql_close(conn);
    free_config(config);
    printf("Database migration successful.\n");
    return 0;
}
