#include "database.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>

MYSQL* db_connect() {
    Config *config = get_config();
    if (config == NULL) {
        return NULL;
    }

    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        free_config(config);
        return NULL;
    }

    if (mysql_real_connect(conn, config->host, config->username, config->password, 
            config->database, atoi(config->port), NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        free_config(config);
        return NULL;
    }

    free_config(config);
    return conn;
}

void db_disconnect(MYSQL *conn) {
    if (conn != NULL) {
        mysql_close(conn);
    }
}

