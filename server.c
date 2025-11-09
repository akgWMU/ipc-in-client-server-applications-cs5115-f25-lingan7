#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define REQUEST_FIFO "/tmp/fifo_request"
#define RESPONSE_FIFO "/tmp/fifo_response"
#define LOG_FILE "server_log.txt"

// Message structure for client-server communication
struct message {
    char operation[4];
    int operand1;
    int operand2;
    pid_t client_pid;  // To identify which client sent the request
};

struct response {
    int result;
    int error;  // 0 = success, 1 = error
    char error_msg[64];
};

// Global file descriptor for cleanup
int request_fd = -1;
FILE *log_file = NULL;
volatile sig_atomic_t keep_running = 1;

// Cleanup function
void cleanup(void) {
    if (request_fd != -1) {
        close(request_fd);
    }
    if (log_file != NULL) {
        fclose(log_file);
    }
    unlink(REQUEST_FIFO);
    unlink(RESPONSE_FIFO);
}

// Signal handler for graceful shutdown
void signal_handler(int signum) {
    printf("\nServer: Received signal %d, shutting down...\n", signum);
    keep_running = 0;
}

// SIGCHLD handler to reap zombie processes
void sigchld_handler(int signum) {
    (void)signum;  // Unused parameter
    // Reap all terminated child processes
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// Process arithmetic operation
struct response process_request(struct message *msg) {
    struct response resp;
    resp.error = 0;
    resp.error_msg[0] = '\0';
    
    if (strcmp(msg->operation, "add") == 0) {
        resp.result = msg->operand1 + msg->operand2;
    } else if (strcmp(msg->operation, "sub") == 0) {
        resp.result = msg->operand1 - msg->operand2;
    } else if (strcmp(msg->operation, "mul") == 0) {
        resp.result = msg->operand1 * msg->operand2;
    } else if (strcmp(msg->operation, "div") == 0) {
        if (msg->operand2 == 0) {
            resp.error = 1;
            strcpy(resp.error_msg, "Division by zero");
        } else {
            resp.result = msg->operand1 / msg->operand2;
        }
    } else {
        resp.error = 1;
        strcpy(resp.error_msg, "Invalid operation");
    }
    
    return resp;
}

// Log server activity
void log_activity(struct message *msg, struct response *resp) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL) return;
    
    fprintf(log, "[PID %d] Client PID: %d | Operation: %s(%d, %d) | ",
            getpid(), msg->client_pid, msg->operation, msg->operand1, msg->operand2);
    
    if (resp->error) {
        fprintf(log, "Error: %s\n", resp->error_msg);
    } else {
        fprintf(log, "Result: %d\n", resp->result);
    }
    fflush(log);
    fclose(log);
}

// Handle client request (runs in child process)
void handle_client(struct message *msg) {
    struct response resp;
    
    printf("Server [PID %d]: Processing request - %s(%d, %d) from Client PID %d\n",
           getpid(), msg->operation, msg->operand1, msg->operand2, msg->client_pid);
    
    // Process the request
    resp = process_request(msg);
    
    // Log the activity
    log_activity(msg, &resp);
    
    // Open response FIFO for writing
    int response_fd = open(RESPONSE_FIFO, O_WRONLY);
    if (response_fd == -1) {
        perror("Server: Error opening response FIFO");
        exit(1);
    }
    
    // Send response back to client
    if (write(response_fd, &resp, sizeof(struct response)) == -1) {
        perror("Server: Error writing response");
    } else {
        if (resp.error) {
            printf("Server [PID %d]: Sent error response: %s\n", getpid(), resp.error_msg);
        } else {
            printf("Server [PID %d]: Sent response: %d\n", getpid(), resp.result);
        }
    }
    
    close(response_fd);
}

int main(void) {
    struct message msg;
    
    // Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGCHLD, sigchld_handler);  // Handle zombie processes
    
    // Open log file to verify it's writable
    log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Server: Error opening log file");
        return 1;
    }
    
    printf("Server: Starting up (PID: %d)...\n", getpid());
    fprintf(log_file, "\n=== Server Started (PID: %d) ===\n", getpid());
    fclose(log_file);
    log_file = NULL;
    
    // Remove old FIFOs if they exist
    unlink(REQUEST_FIFO);
    unlink(RESPONSE_FIFO);
    
    // Create named pipes
    if (mkfifo(REQUEST_FIFO, 0666) == -1) {
        perror("Server: Error creating request FIFO");
        return 1;
    }
    
    if (mkfifo(RESPONSE_FIFO, 0666) == -1) {
        perror("Server: Error creating response FIFO");
        unlink(REQUEST_FIFO);
        return 1;
    }
    
    printf("Server: FIFOs created successfully\n");
    printf("Server: Waiting for client requests... (Press Ctrl+C to stop)\n");
    printf("Server: Using fork() to handle multiple clients concurrently\n\n");
    
    // Main server loop
    while (keep_running) {
        // Open request FIFO for reading (blocks until client connects)
        request_fd = open(REQUEST_FIFO, O_RDONLY);
        if (request_fd == -1) {
            if (errno == EINTR) {
                // Interrupted by signal, check if we should continue
                continue;
            }
            perror("Server: Error opening request FIFO");
            cleanup();
            return 1;
        }
        
        // Read request from client
        ssize_t bytes_read = read(request_fd, &msg, sizeof(struct message));
        
        if (bytes_read == sizeof(struct message)) {
            printf("Server [Parent PID %d]: Received request from Client PID %d\n",
                   getpid(), msg.client_pid);
            
            // Fork a child process to handle this client
            pid_t pid = fork();
            
            if (pid < 0) {
                // Fork failed
                perror("Server: Fork failed");
                close(request_fd);
                continue;
            } 
            else if (pid == 0) {
                // Child process: handle the client request
                close(request_fd);  // Child doesn't need the request fd anymore
                handle_client(&msg);
                exit(0);  // Child exits after handling request
            } 
            else {
                // Parent process: continue accepting new clients
                printf("Server [Parent PID %d]: Forked child PID %d to handle client\n",
                       getpid(), pid);
                close(request_fd);
                request_fd = -1;
            }
        } else if (bytes_read == 0) {
            // Client closed connection, wait for next client
            printf("Server: Client disconnected, waiting for next request...\n");
            close(request_fd);
            request_fd = -1;
        } else {
            if (errno != EINTR) {
                perror("Server: Error reading request");
            }
            close(request_fd);
            request_fd = -1;
        }
    }
    
    printf("\nServer: Cleaning up and exiting...\n");
    
    // Wait for any remaining child processes
    printf("Server: Waiting for child processes to complete...\n");
    while (waitpid(-1, NULL, WNOHANG) > 0);
    
    cleanup();
    printf("Server: Shutdown complete\n");
    return 0;
}
