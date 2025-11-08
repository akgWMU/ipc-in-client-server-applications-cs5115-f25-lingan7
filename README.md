# Client-Server Arithmetic Calculator

## CS5115 - Programming Assignment 6

**Author:** Lingamuthu Kalyansundaram  
**Date:** Fall 2025

---

## Overview

This project implements a client-server architecture in C using Named Pipes (FIFOs) for interprocess communication. The server performs arithmetic operations (addition, subtraction, multiplication, division) based on client requests and returns results.

---

## IPC Approach: Named Pipes (FIFOs)

**Choice Rationale:**
- Named pipes provide a simple, file-system-based IPC mechanism
- Well-suited for unidirectional communication patterns
- Easy to implement and debug
- Appropriate for educational purposes and demonstrating IPC fundamentals

**Implementation Details:**
- Two FIFOs are created: `/tmp/fifo_request` (client → server) and `/tmp/fifo_response` (server → client)
- The server opens the request FIFO in blocking mode, waiting for client connections
- Clients send structured messages containing operation type and operands
- The server processes requests, logs activity, and sends responses back through the response FIFO

---

## Architecture

### Message Structure
```c
struct message {
    char operation[4];    // "add", "sub", "mul", "div"
    int operand1;         // First operand
    int operand2;         // Second operand
    pid_t client_pid;     // Client process ID for logging
};

struct response {
    int result;           // Calculation result
    int error;            // Error flag (0 = success, 1 = error)
    char error_msg[64];   // Error description
};
```

### Server Features
- Creates and manages named pipes
- **Uses fork() to handle multiple concurrent client connections**
- **Each client request is handled by a child process**
- Validates operations and checks for division by zero
- Logs all transactions to `server_log.txt`
- Graceful shutdown with signal handlers (SIGINT, SIGTERM, SIGCHLD)
- Automatic cleanup of FIFOs and zombie processes on exit

### Client Features
- Interactive command-line interface
- Input validation for operations and operands
- Clear error reporting
- Graceful exit with "exit" command

---

## Compilation and Execution

### Prerequisites
- GCC compiler
- UNIX/Linux environment
- Standard C libraries

### Build Instructions

```bash
# Compile both server and client
make

# Or compile individually
make server
make client
```

### Running the Application

**Terminal 1 - Start Server:**
```bash
./server
```

**Terminal 2 - Run Client:**
```bash
./client
```

### Example Session

```
Client: Enter operation (add/sub/mul/div): mul
Client: Enter operands (two integers): 6 9
Client: Result from server: 54

Client: Enter operation (add/sub/mul/div): div
Client: Enter operands (two integers): 10 0
Client: Error from server: Division by zero

Client: Enter operation (add/sub/mul/div): exit
Client: Exiting...
```

### Stopping the Server

**Option 1:** Use Ctrl+C in the server terminal  
**Option 2:** Use make command:
```bash
make stop
```

---

## Makefile Targets

| Target | Description |
|--------|-------------|
| `make` or `make all` | Compile both server and client |
| `make server` | Compile server only |
| `make client` | Compile client only |
| `make run_server` | Compile and run server in background |
| `make run_client` | Compile and run client |
| `make test` | Start server for testing |
| `make stop` | Stop running server |
| `make clean` | Remove executables, FIFOs, and logs |
| `make distclean` | Full cleanup including stopping server |

---

## File Structure

```
.
├── server.c           # Server implementation
├── client.c           # Client implementation
├── Makefile           # Build configuration
├── README.md          # This file
└── server_log.txt     # Generated log file (created at runtime)
```

---

## Error Handling

### Server-Side
- Validates operation types
- Detects division by zero
- Handles FIFO creation failures
- Logs all errors
- Graceful shutdown on signals

### Client-Side
- Validates user input
- Checks for valid operations
- Detects server availability
- Handles FIFO communication errors
- Input buffer clearing on invalid input

---

## Testing Recommendations

### Test Cases

1. **Basic Operations**
   - Test each operation: add, sub, mul, div
   - Verify correct results

2. **Edge Cases**
   - Division by zero
   - Large numbers (integer overflow)
   - Negative numbers

3. **Invalid Input**
   - Invalid operation names
   - Non-integer operands
   - Empty input

4. **Multiple Concurrent Clients**
   - Run multiple client instances simultaneously
   - Verify each client gets correct response
   - Check server creates child processes (use `ps aux | grep server`)

5. **Server Robustness**
   - Test server restart
   - Test graceful shutdown (Ctrl+C)
   - Verify cleanup of FIFOs and zombie processes

---

## Known Limitations and Assumptions

### Limitations
1. **Sequential Processing:** Server handles one client request at a time (no concurrent processing)
2. **Integer Arithmetic Only:** Limited to integer operations (no floating-point support)
3. **Integer Division:** Division truncates results to integers
4. **FIFO Blocking:** Clients block while waiting for server responses
5. **No Authentication:** No client verification or security measures

### Assumptions
1. Server must be started before clients
2. FIFOs are created in `/tmp/` directory (requires write permissions)
3. Clients must properly close connections
4. One client communicates with server at a time
5. Standard input is available for client interaction

---

## Design Decisions

### Why Named Pipes?
- **Simplicity:** Easier to understand and implement than sockets
- **File-System Based:** Visible in filesystem for debugging
- **Blocking Semantics:** Natural synchronization between client and server
- **Educational Value:** Clear demonstration of IPC concepts

### Why Two FIFOs?
- Separate request and response channels avoid message collision
- Simpler logic than bidirectional communication
- Clear unidirectional data flow

### Why Process ID in Messages?
- Enables server logging of which client made each request
- Useful for debugging multiple client scenarios
- Provides traceability in logs

---

## Future Enhancements

Potential improvements for this project:
1. **Concurrent Server:** Use fork() or threads to handle multiple clients simultaneously
2. **Floating-Point Support:** Add support for decimal operations
3. **Socket Implementation:** Migrate to TCP/UDP sockets for network capability
4. **More Operations:** Add modulo, power, square root, etc.
5. **Client Authentication:** Add simple authentication mechanism
6. **Configuration File:** Allow customizable FIFO paths and log locations

---

## References

- UNIX System Programming (Stevens & Rago)
- Linux man pages: `mkfifo(3)`, `open(2)`, `read(2)`, `write(2)`
- CS5115 Course Materials

---

## Troubleshooting

### "Error opening request FIFO"
- Ensure server is running first
- Check `/tmp/` directory permissions
- Verify FIFOs exist: `ls -l /tmp/fifo_*`

### "Permission denied"
- Check FIFO permissions (should be 0666)
- Ensure you have write access to `/tmp/`

### Server doesn't stop with Ctrl+C
- Use: `make stop` or `pkill server`

### Stale FIFOs after crash
- Run: `make clean` to remove old FIFOs

---

## License

This project is created for educational purposes as part of CS5115 coursework.
