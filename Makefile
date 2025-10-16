CC = gcc
CFLAGS = -Isrc -Wall -Wextra -std=c11 `mysql_config --cflags`
LDFLAGS = `mysql_config --libs`

BUILD_DIR = build

# Source files with new structure
SERVER_SRC = src/main.c \
             src/core/server.c \
             src/core/config.c \
             src/core/database.c \
             src/routes/routes.c \
             src/routes/health_routes.c \
             src/routes/notes_routes.c \
             src/handlers/health_handler.c \
             src/handlers/notes_handler.c \
             src/services/notes_service.c \
             src/repositories/notes_repository.c

SERVER_OBJ = $(BUILD_DIR)/main.o \
             $(BUILD_DIR)/core/server.o \
             $(BUILD_DIR)/core/config.o \
             $(BUILD_DIR)/core/database.o \
             $(BUILD_DIR)/routes/routes.o \
             $(BUILD_DIR)/routes/health_routes.o \
             $(BUILD_DIR)/routes/notes_routes.o \
             $(BUILD_DIR)/handlers/health_handler.o \
             $(BUILD_DIR)/handlers/notes_handler.o \
             $(BUILD_DIR)/services/notes_service.o \
             $(BUILD_DIR)/repositories/notes_repository.o

MIGRATE_SRC = db/migrate.c src/core/config.c
MIGRATE_OBJ = $(BUILD_DIR)/db/migrate.o $(BUILD_DIR)/core/config.o

all: build

build: $(BUILD_DIR)/server
	@echo "App Successfully Built"

$(BUILD_DIR)/server: $(SERVER_OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Main source file
$(BUILD_DIR)/main.o: src/main.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Core module
$(BUILD_DIR)/core/%.o: src/core/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Routes module
$(BUILD_DIR)/routes/%.o: src/routes/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Handlers module
$(BUILD_DIR)/handlers/%.o: src/handlers/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Services module
$(BUILD_DIR)/services/%.o: src/services/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Repositories module
$(BUILD_DIR)/repositories/%.o: src/repositories/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Database migrations
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
