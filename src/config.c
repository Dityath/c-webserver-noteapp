#define _POSIX_C_SOURCE 200809L
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config *get_config() {
  FILE *file = fopen(".env", "r");
  if (file == NULL) {
    perror("Error opening .env file");
    return NULL;
  }

  Config *config = malloc(sizeof(Config));
  if (config == NULL) {
    perror("Error allocating memory for config");
    fclose(file);
    return NULL;
  }

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    char *key = strtok(line, "=");
    char *value = strtok(NULL, "\n");
    if (key != NULL && value != NULL) {
      if (strcmp(key, "HOST") == 0) {
        config->host = strdup(value);
      } else if (strcmp(key, "PORT") == 0) {
        config->port = strdup(value);
      } else if (strcmp(key, "USERNAME") == 0) {
        config->username = strdup(value);
      } else if (strcmp(key, "PASSWORD") == 0) {
        config->password = strdup(value);
      } else if (strcmp(key, "DATABASE") == 0) {
        config->database = strdup(value);
      }
    }
  }

  fclose(file);
  return config;
}

void free_config(Config *config) {
  if (config != NULL) {
    free(config->host);
    free(config->port);
    free(config->username);
    free(config->password);
    free(config->database);
    free(config);
  }
}
