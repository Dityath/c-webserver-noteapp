#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
  char *host;
  char *port;
  char *username;
  char *password;
  char *database;
} Config;

Config *get_config();
void free_config(Config *);

#endif
