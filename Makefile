CC = gcc
CFLAGS = -Isrc -Wall -Wextra -std=c11 `mysql_config --cflags`
LDFLAGS = `mysql_config --libs`

BUILD_DIR = build

SERVER_SRC = src/main.c src/server.c src/routes.c src/handlers.c src/services.c src/repositories.c src/config.c src/database.c
SERVER_OBJ = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SERVER_SRC))

MIGRATE_SRC = db/migrate.c src/config.c
MIGRATE_OBJ = $(patsubst db/%.c, $(BUILD_DIR)/db/%.o, $(filter db/%.c, $(MIGRATE_SRC))) \
             $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(filter src/%.c, $(MIGRATE_SRC)))

all: build

build: $(BUILD_DIR)/server
	@echo "App Successfully Built"

$(BUILD_DIR)/server: $(SERVER_OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/db/%.o: db/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<


migrate: $(BUILD_DIR)/migrate
	./$(BUILD_DIR)/migrate

$(BUILD_DIR)/migrate: $(MIGRATE_OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

start: $(BUILD_DIR)/server
	@echo "Server started on http://localhost:8000"
	./$(BUILD_DIR)/server

clean:
	rm -rf $(BUILD_DIR)
