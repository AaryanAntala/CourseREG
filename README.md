# Academia Portal (Mini Project)

Multithreaded TCP client-server system in C providing role-based academic management (Admin / Faculty / Student) with persistent storage and concurrency control.

## Features
- Admin: add faculty/student, toggle student activation, update username/password, list users, view courses.
- Faculty: add/remove course, view own courses, view enrollments (with student list), change password.
- Student: enroll / unenroll, view enrolled courses, list available courses, change password.
- Persistent storage: users, courses, enrollments saved to text files.
- Concurrency: one thread per client, semaphore-protected saves, per-file read/write locks (`flock`).
- Graceful shutdown via signal handler (saves data, frees memory).

## Tech Stack
- C (POSIX)
- Sockets (`AF_INET`, TCP)
- Threads (`pthread`)
- Synchronization: `sem_t`, `flock`
- Dynamic arrays with `realloc`
- Text file persistence

## Build
From project root:
```
gcc -w -pthread -o server server.c
gcc -o client client.c
```

## Run
1. Start server (default port 8080):
```
./server
```
2. Start one or more clients:
```
./client
```

## Initial Credentials
```
Username: admin
Password: admin123
```

## Protocol Overview
Client sends single-line commands. Server replies with text response (no binary framing).

### Login
```
LOGIN <username> <password>
```
Success: `LOGIN_SUCCESS <ROLE> <USER_ID>`  
Failure: `LOGIN_FAILED <reason>`

After login send: `<ROLE> <USER_ID> <COMMAND...>`

### Admin Commands
```
ADMIN <id> ADD_STUDENT <username> <password>
ADMIN <id> ADD_FACULTY <username> <password>
ADMIN <id> TOGGLE_STUDENT <studentId>
ADMIN <id> UPDATE_USER <userId> username|password <value>
ADMIN <id> VIEW_USERS
ADMIN <id> VIEW_COURSES
```

### Faculty Commands
```
FACULTY <id> ADD_COURSE <code> <seats> <course name...>
FACULTY <id> REMOVE_COURSE <code>
FACULTY <id> VIEW_COURSES
FACULTY <id> VIEW_ENROLLMENTS
FACULTY <id> CHANGE_PASSWORD <old> <new>
```

### Student Commands
```
STUDENT <id> ENROLL <courseCode>
STUDENT <id> UNENROLL <courseCode>
STUDENT <id> VIEW_ENROLLED
STUDENT <id> VIEW_COURSES
STUDENT <id> CHANGE_PASSWORD <old> <new>
```

### Exit
Client may send:
```
EXIT
```

## Concurrency & Consistency
- Each client handled in its own thread (`handleClient`).
- `saveData()` guarded by semaphore to serialize disk writes.
- Read vs write access to course/enrollment files distinguished by `LOCK_SH` / `LOCK_EX`.
- In-memory arrays updated atomically within request handling path before save.

## Data Files
```
users.txt
courses.txt
enrollments.txt
```
Plain text; regenerated fully on each `saveData()`.

## Error Handling
- Basic format validation per command.
- Permission checks (role + ownership).
- Course capacity and duplicate enrollment checks.


## Possible Improvements
- Replace text files with SQLite.
- Add password hashing (e.g., bcrypt).
- Introduce structured message framing or JSON.
- Add logging subsystem.
- Improve error codes vs plain text.
- Add unit tests and integration tests.

## Signals
On termination signals (e.g., Ctrl+C), server:
- Saves data
- Destroys semaphore
- Frees dynamic arrays
- Exits cleanly

## Folder Layout (key files)
```
server.c
client.c
cmd.txt
(users.txt / courses.txt / enrollments.txt created at runtime)
```


