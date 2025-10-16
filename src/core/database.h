#ifndef DATABASE_H
#define DATABASE_H

#include <mysql/mysql.h>

MYSQL *db_connect();
void db_disconnect(MYSQL *);

#endif
