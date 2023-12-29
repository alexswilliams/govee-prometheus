#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <asm-generic/errno-base.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "interrupts.h"
#include "metrics.h"


static char *build_metrics_response() {
    char *const metrics = build_metrics();
    if (metrics == NULL) {
        return strdup("HTTP/1.0 500 Internal Server Error\nConnection: Close\n\n");
    }
    const size_t buf_size = strlen(metrics) + 200;
    char *const buf = malloc(buf_size);
    snprintf(buf, buf_size,
             "HTTP/1.0 200 OK\n"
             "Connection: Close\n"
             "Content-Type: text/plain\n"
             "Content-Length: %lu\n"
             "\n"
             "%s",
             strlen(metrics), metrics);
    free(metrics);
    return buf;
}

static void respond_with_404(const int client_fd) {
    const char *response = "HTTP/1.0 404 Not Found\n"
            "Connection: Close\n"
            "Content-Length: 0\n"
            "\n";
    send(client_fd, response, strlen(response), 0);
}

#define RECEIVE_BUFFER_SIZE 1024

static void handle_incoming_request(const int client_fd) {
    char receive_buffer[RECEIVE_BUFFER_SIZE];
    const ssize_t bytes_received = recv(client_fd, receive_buffer, sizeof(receive_buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("Error while receiving HTTP payload");
        return;
    }
    receive_buffer[bytes_received] = 0;
    if (strncmp(receive_buffer, "GET /favicon.ico", bytes_received) == 0) {
        respond_with_404(client_fd);
        return;
    }
    if (strncmp(receive_buffer, "GET /metrics HTTP/1", 19) != 0) {
        fprintf(stderr, "Invalid request received: \n%s\n", receive_buffer);
        fflush(stderr);
        respond_with_404(client_fd);
        return;
    }
    char *const metrics = build_metrics_response();
    send(client_fd, metrics, strlen(metrics), 0);
    free(metrics);
}

static void accept_clients(const int server_fd) {
    while (!exit_requested()) {
        int client_fd;
        while ((client_fd = accept(server_fd, NULL, NULL)) < 0) {
            if (errno == EINTR && exit_requested()) return;
            if (errno == EAGAIN || errno == EINTR) {
                usleep(100);
                continue;
            }
            perror("Could not accept connection");
            return;
        }

        handle_incoming_request(client_fd);
        close(client_fd);
    }
}

void prom_thread_entrypoint() {
    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Couldn't create server socket");
        return;
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    if (bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Could not bind to port 8080");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 1) < 0) {
        perror("Could not listen to bound server socket");
        close(server_fd);
        return;
    }

    accept_clients(server_fd);

    close(server_fd);
}
