# Makefile for Client-Server Arithmetic Calculator
# CS5115 PA6

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
TARGETS = server client

.PHONY: all clean run_server run_client test

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Run the server in the background
run_server: server
	@echo "Starting server..."
	@./server > server_output.log 2>&1 &
	@echo $! > .server.pid
	@sleep 1
	@echo "Server started (PID: $(cat .server.pid 2>/dev/null || echo 'unknown'))"

# Run a client instance
run_client: client
	./client

# Run a quick test
test: all
	@echo "Starting server..."
	@./server &
	@sleep 1
	@echo "Server started. To test, run: make run_client in another terminal"
	@echo "To stop server: make stop"

# Stop the server
stop:
	@if [ -f .server.pid ]; then \
		PID=$(cat .server.pid); \
		if kill -0 $PID 2>/dev/null; then \
			echo "Stopping server (PID: $PID)..."; \
			kill -SIGINT $PID; \
			sleep 1; \
		fi; \
		rm -f .server.pid; \
	else \
		pkill -SIGINT server 2>/dev/null || echo "No server process found"; \
	fi

# Clean up executables, FIFOs, and log file
clean:
	rm -f $(TARGETS)
	rm -f /tmp/fifo_request /tmp/fifo_response
	rm -f server_log.txt server_output.log .server.pid
	@echo "Cleaned up executables, FIFOs, and log files"

# Full cleanup including stopping server
distclean: clean stop