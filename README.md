# Multi-Process HTTP Web Server

A minimal HTTP-style file server written in C on raw POSIX sockets — no frameworks, no libraries beyond libc. The server handles concurrency with a **fork-per-connection process model**: every accepted connection is served by its own child process, while the parent immediately returns to `accept()`.

Built as a Unix systems programming project at Cal Poly (CSC 357). Includes a small interactive TCP client.

## Architecture

```
            ┌────────────┐
   accept() │   parent   │  loops on accept(), never blocks on I/O
            └─────┬──────┘
                  │ fork() per connection
      ┌───────────┼───────────┐
 ┌────▼───┐  ┌────▼───┐  ┌────▼───┐
 │ child  │  │ child  │  │ child  │   each: parse GET → open file →
 └────────┘  └────────┘  └────────┘   stream contents → exit(0)
```

- **`net.c`** — socket lifecycle: `create_service()` builds the listening socket (`socket` → `SO_REUSEADDR` → `bind` → `listen`, backlog 50); `accept_connection()` wraps `accept()` with `EINTR`-safe retry.
- **`server.c`** — the process model: forks a child per connection, and installs a `SIGCHLD` handler that reaps zombies with a non-blocking `waitpid(WNOHANG)` loop so long-running service never accumulates dead children.
- **`client.c`** — interactive client: resolves the host, connects, forwards stdin lines as requests, and prints responses.

## Build & run

```bash
make            # builds ./server and ./client
./server        # listens on port 4000
```

Request a file (the protocol is a simplified `GET <path>`):

```bash
printf 'GET source.txt\n' | nc localhost 4000
# Hello
# This is a test file.
```

Or use the included client interactively:

```bash
./client localhost
GET source.txt
```

## Notes & limitations

- The request grammar is a course-simplified subset of HTTP (`GET <path>`, no headers or status lines).
- Paths are opened as given, relative to the server's working directory — run it only in a directory you intend to serve, on a trusted network. There is no path sanitization; that (plus status codes and MIME types) is the natural next step.
- Fork-per-connection trades throughput for isolation and simplicity: a crashed handler kills one child, never the server.
