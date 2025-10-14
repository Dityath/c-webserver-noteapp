# C Web Server

A simple web server written in C for a note-taking application.

## Prerequisites

- A C compiler (e.g., gcc)
- Make
- MySQL server
- `libmysqlclient-dev`

**Ubuntu/Debian:**

```bash
sudo apt-get install libmysqlclient-dev
```

**Fedora/CentOS/RHEL:**

```bash
sudo dnf install mysql-devel
```

## Setup

1. Create a `.env` file in the root of the project with the following content:

    ```
    # mysql database
    HOST=localhost
    PORT=3306
    USERNAME=your_username
    PASSWORD=your_password
    DATABASE=note_app_on_c
    ```

2. Create the database `note_app_on_c` in your MySQL server.

## Build and Run

- **Build the server:**

  ```bash
  make build
  ```

- **Run database migrations:**

  ```bash
  make migrate
  ```

- **Start the server:**

  ```bash
  make start
  ```

- **Clean up build artifacts:**

  ```bash
  make clean
  ```

The server will start on `http://localhost:8000`.

## API Documentation

### Health Check

- **GET /api/health**

  Returns the status of the server.

  **Curl:**

  ```bash
  curl http://localhost:8000/api/health
  ```

### Notes

- **GET /api/notes**

  Returns a list of all notes.

  **Curl:**

  ```bash
  curl http://localhost:8000/api/notes
  ```

- **GET /api/notes/:id**

  Returns a single note by its ID.

  **Curl:**

  ```bash
  curl http://localhost:8000/api/notes/1
  ```

- **POST /api/notes**

  Creates a new note.

  **Curl:**

  ```bash
  curl -X POST -H "Content-Type: application/json" -d '{"title":"My Note", "content":"This is my note."}' http://localhost:8000/api/notes
  ```

- **PUT /api/notes/:id**

  Updates a note by its ID.

  **Curl:**

  ```bash
  curl -X PUT -H "Content-Type: application/json" -d '{"title":"Updated Note", "content":"This is my updated note."}' http://localhost:8000/api/notes/1
  ```

- **DELETE /api/notes/:id**

  Deletes a note by its ID.

  **Curl:**

  ```bash
  curl -X DELETE http://localhost:8000/api/notes/1
  ```
