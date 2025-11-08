# Client-Server Arithmetic Calculator Using IPC

[![C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![UNIX](https://img.shields.io/badge/platform-UNIX%2FLinux-lightgrey.svg)](https://www.unix.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

A client-server application demonstrating **Interprocess Communication (IPC)** using Named Pipes (FIFOs) in C. Built for CS5115 Programming Assignment 6.

## 📋 Overview

This project implements a multi-process architecture where:
- A **server** process handles arithmetic operations
- **Client** processes send requests and receive results
- Communication occurs via **Named Pipes (FIFOs)**
- All operations are logged for traceability

## ✨ Features

- ✅ **Four arithmetic operations**: Addition, Subtraction, Multiplication, Division
- ✅ **Named Pipe IPC**: Uses UNIX FIFOs for process communication
- ✅ **Concurrent client handling**: Server uses `fork()` to handle multiple clients simultaneously
- ✅ **Process management**: Automatic zombie process reaping with SIGCHLD handler
- ✅ **Error handling**: Division by zero detection, input validation
- ✅ **Activity logging**: Complete server transaction history with process IDs
- ✅ **Graceful shutdown**: Signal handlers for clean termination
- ✅ **Resource cleanup**: Proper FIFO and file descriptor management

## 🏗️ Architecture

```
┌─────────┐                    ┌─────────────────┐
│ Client  │  ─── Request ───>  │ Server (Parent) │
│ Process │                    │   PID: 1234     │
│         │  <── Response ───  └────────┬────────┘
└─────────┘                             │ fork()
              (Named Pipes)             │
                                   ┌────▼────┐
┌─────────┐                       │  Child  │
│ Client  │  ─── Request ───>    │ PID: 1235│
│ Process │                       └─────────┘
│         │  <── Response ───          
└─────────┘                       
```

**Communication Flow:**
1. Client sends operation + operands via `/tmp/fifo_request`
2. Server parent process receives request and forks a child
3. Child process handles computation and logs to `server_log.txt`
4. Child returns result via `/tmp/fifo_response` and exits
5. Parent server continues accepting new clients

## 🚀 Quick Start

### Prerequisites

- GCC compiler
- UNIX/Linux environment (Linux, macOS, WSL)
- Write access to `/tmp/` directory

### Installation

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/ipc-arithmetic-calculator.git
cd ipc-arithmetic-calculator

# Compile the project
make
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

### Example Usage

```
Client: Enter operation (add/sub/mul/div): mul
Client: Enter operands (two integers): 6 9
Client: Result from server: 54

Client: Enter operation (add/sub/mul/div): div
Client: Enter operands (two integers): 10 2
Client: Result from server: 5

Client: Enter operation (add/sub/mul/div): exit
Client: Exiting...
```

## 📁 Project Structure

```
.
├── server.c           # Server implementation
├── client.c           # Client implementation
├── Makefile           # Build configuration
├── README.md          # This file
├── TECHNICAL_REPORT.md # Detailed technical documentation
├── .gitignore         # Git ignore rules
└── server_log.txt     # Generated at runtime
```

## 🛠️ Makefile Commands

| Command | Description |
|---------|-------------|
| `make` | Compile both server and client |
| `make server` | Compile server only |
| `make client` | Compile client only |
| `make run_server` | Start server in background |
| `make run_client` | Run client interactively |
| `make clean` | Remove executables and logs |
| `make stop` | Stop running server |

## 🧪 Testing

### Basic Functionality Tests
```bash
# Test all operations
./client
# Try: add 5 3, sub 10 4, mul 6 9, div 20 5

# Test error cases
# Try: div 10 0 (division by zero)
# Try: xyz 1 2 (invalid operation)
```

### Multiple Clients
```bash
# Terminal 1: Server
./server

# Terminal 2-4: Multiple clients running concurrently
./client  # Each in separate terminal

# Monitor server processes
ps aux | grep server  # Shows parent + child processes
```

### View Server Logs
```bash
cat server_log.txt
```

## 🔧 Technical Details

### IPC Mechanism
- **Type**: Named Pipes (FIFOs)
- **Request FIFO**: `/tmp/fifo_request`
- **Response FIFO**: `/tmp/fifo_response`
- **Synchronization**: Blocking I/O operations

### Message Protocol

```c
// Request structure
struct message {
    char operation[4];    // "add", "sub", "mul", "div"
    int operand1;
    int operand2;
    pid_t client_pid;
};

// Response structure
struct response {
    int result;
    int error;            // 0 = success, 1 = error
    char error_msg[64];
};
```

## ⚠️ Known Limitations

- **Sequential Processing**: Server handles one client at a time
- **Integer Operations**: No floating-point support
- **Local Only**: No network communication (UNIX domain only)
- **No Authentication**: No client verification

## 🐛 Troubleshooting

### "Error opening request FIFO"
**Solution:** Ensure server is running first
```bash
./server &
./client
```

### "Permission denied"
**Solution:** Check `/tmp/` write permissions
```bash
ls -ld /tmp
```

### Server won't stop
**Solution:** Use make command or kill manually
```bash
make stop
# OR
pkill server
```

### Stale FIFOs after crash
**Solution:** Clean up manually
```bash
make clean
```

## 📚 Documentation

- **[Technical Report](TECHNICAL_REPORT.md)** - Detailed design decisions and testing results
- **Code Comments** - Inline documentation in source files

## 🎓 Educational Context

**Course:** CS5115 - Programming Prep for Graduate Students  
**Assignment:** PA6 - Client-Server Communication Using IPC  
**Semester:** Fall 2025  
**Author:** Lingamuthu Kalyansundaram

### Learning Objectives Covered
- ✅ Process creation with `fork()`
- ✅ Parent-child process relationships
- ✅ Interprocess communication via Named Pipes
- ✅ Request-response protocol design
- ✅ Concurrent process management
- ✅ Zombie process prevention with `waitpid()`
- ✅ Signal handling (SIGCHLD, SIGINT, SIGTERM)
- ✅ Error handling and resource management

## 🤝 Contributing

This is an academic project, but suggestions are welcome:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/improvement`)
3. Commit changes (`git commit -m 'Add improvement'`)
4. Push to branch (`git push origin feature/improvement`)
5. Open a Pull Request

## 📝 License

This project is created for educational purposes. See [LICENSE](LICENSE) for details.

## 🔗 References

- Stevens & Rago - *Advanced Programming in the UNIX Environment*
- Linux man pages: `mkfifo(3)`, `open(2)`, `read(2)`, `write(2)`
- CS5115 Course Materials

## 📧 Contact

**Lingamuthu Kalyansundaram**  
CS5115 - Fall 2025

---

⭐ If you found this helpful, please star the repository!