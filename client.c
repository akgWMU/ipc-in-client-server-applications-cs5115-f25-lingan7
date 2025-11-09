#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define REQUEST_FIFO "/tmp/fifo_request"
#define RESPONSE_FIFO "/tmp/fifo_response"

// Message structure (must match server)
struct message {
    char operation[4];
    int operand1;
    int operand2;
    pid_t client_pid;
};

struct response {
    int result;
    int error;
    char error_msg[64];
};

// Function to validate operation input
int is_valid_operation(const char *op) {
    return (strcmp(op, "add") == 0 || 
            strcmp(op, "sub") == 0 || 
            strcmp(op, "mul") == 0 || 
            strcmp(op, "div") == 0);
}

// Function to send request and receive response
int communicate_with_server(struct message *msg) {
    int request_fd, response_fd;
    struct response resp;
    
    // Open request FIFO for writing
    request_fd = open(REQUEST_FIFO, O_WRONLY);
    if (request_fd == -1) {
        perror("Client: Error opening request FIFO");
        printf("Client: Is the server running?\n");
        return -1;
    }
    
    // Send request to server
    if (write(request_fd, msg, sizeof(struct message)) == -1) {
        perror("Client: Error sending request");
        close(request_fd);
        return -1;
    }
    
    close(request_fd);
    
    // Open response FIFO for reading
    response_fd = open(RESPONSE_FIFO, O_RDONLY);
    if (response_fd == -1) {
        perror("Client: Error opening response FIFO");
        return -1;
    }
    
    // Read response from server
    ssize_t bytes_read = read(response_fd, &resp, sizeof(struct response));
    if (bytes_read != sizeof(struct response)) {
        perror("Client: Error reading response");
        close(response_fd);
        return -1;
    }
    
    close(response_fd);
    
    // Display result
    if (resp.error) {
        printf("Client: Error from server: %s\n", resp.error_msg);
    } else {
        printf("Client: Result from server: %d\n", resp.result);
    }
    
    return 0;
}

int main(void) {
    struct message msg;
    char operation[10];
    
    printf("Client: Connected to arithmetic server (PID: %d)\n", getpid());
    printf("Client: Available operations: add, sub, mul, div\n");
    printf("Client: Type 'exit' to quit\n\n");
    
    msg.client_pid = getpid();
    
    while (1) {
        // Get operation from user
        printf("Client: Enter operation (add/sub/mul/div): ");
        if (scanf("%9s", operation) != 1) {
            printf("Client: Invalid input\n");
            while (getchar() != '\n');  // Clear input buffer
            continue;
        }
        
        // Check for exit command
        if (strcmp(operation, "exit") == 0) {
            printf("Client: Exiting...\n");
            break;
        }
        
        // Validate operation
        if (!is_valid_operation(operation)) {
            printf("Client: Invalid operation. Use add, sub, mul, or div\n");
            continue;
        }
        
        // Copy operation to message
        strncpy(msg.operation, operation, 3);
        msg.operation[3] = '\0';
        
        // Get operands from user
        printf("Client: Enter operands (two integers): ");
        if (scanf("%d %d", &msg.operand1, &msg.operand2) != 2) {
            printf("Client: Invalid operands. Please enter two integers.\n");
            while (getchar() != '\n');  // Clear input buffer
            continue;
        }
        
        // Send request and receive response
        if (communicate_with_server(&msg) == -1) {
            printf("Client: Communication with server failed\n");
            break;
        }
        
        printf("\n");
    }
    
    return 0;
}
