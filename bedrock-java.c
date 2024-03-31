#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define CONFIG_FILE "config.conf"
#define TCP_PORT 25565 // Minecraft default TCP port
#define UDP_PORT 19132 // Minecraft default UDP port

void handle_tcp_connection(int client_socket, const char *motd) {
    char buffer[1024];
    ssize_t bytes_received;

    // Send MOTD to client
    send(client_socket, motd, strlen(motd), 0);

    // Receive data from client and echo it back
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        send(client_socket, buffer, bytes_received, 0);
    }

    if (bytes_received == -1) {
        perror("TCP receive error");
    }

    close(client_socket);
}

void handle_udp_connection(int server_socket, const char *motd) {
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    ssize_t bytes_received;

    // Receive data from client
    bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);

    // Send MOTD to client
    sendto(server_socket, motd, strlen(motd), 0, (struct sockaddr *)&client_addr, addr_len);

    if (bytes_received == -1) {
        perror("UDP receive error");
    }

    close(server_socket);
}

int main() {
    // Read server configuration from config.conf
    FILE *config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL) {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }

    char tcp_server_ip[100], udp_server_ip[100], motd[1024];
    while (fscanf(config_file, "%*[^=]=%99s", tcp_server_ip) != EOF) {
        fscanf(config_file, "%*[^=]=%99s", udp_server_ip);
        fscanf(config_file, "%*[^=]=%1023[^\n]", motd);
    }

    fclose(config_file);

    // TCP server setup
    int tcp_server_socket;
    struct sockaddr_in tcp_server_addr;
    if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("TCP socket creation error");
        exit(EXIT_FAILURE);
    }

    tcp_server_addr.sin_family = AF_INET;
    tcp_server_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_server_addr.sin_port = htons(TCP_PORT);

    if (bind(tcp_server_socket, (struct sockaddr *)&tcp_server_addr, sizeof(tcp_server_addr)) == -1) {
        perror("TCP bind error");
        close(tcp_server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(tcp_server_socket, 5) == -1) {
        perror("TCP listen error");
        close(tcp_server_socket);
        exit(EXIT_FAILURE);
    }

    printf("TCP server listening on port %d...\n", TCP_PORT);

    // UDP server setup
    int udp_server_socket;
    struct sockaddr_in udp_server_addr;
    if ((udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("UDP socket creation error");
        close(tcp_server_socket);
        exit(EXIT_FAILURE);
    }

    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_addr.s_addr = INADDR_ANY;
    udp_server_addr.sin_port = htons(UDP_PORT);

    if (bind(udp_server_socket, (struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr)) == -1) {
        perror("UDP bind error");
        close(tcp_server_socket);
        close(udp_server_socket);
        exit(EXIT_FAILURE);
    }

    printf("UDP server listening on port %d...\n", UDP_PORT);

    // Main loop to accept connections
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds); // Clear the file descriptor set
        FD_SET(tcp_server_socket, &read_fds);
        FD_SET(udp_server_socket, &read_fds);

        int max_fd = (tcp_server_socket > udp_server_socket) ? tcp_server_socket : udp_server_socket;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Select error");
            continue; // Continue listening for connections
        }

        // Handle TCP connections
        if (FD_ISSET(tcp_server_socket, &read_fds)) {
            int tcp_client_socket;
            struct sockaddr_in tcp_client_addr;
            socklen_t tcp_client_len = sizeof(tcp_client_addr);
            if ((tcp_client_socket = accept(tcp_server_socket, (struct sockaddr *)&tcp_client_addr, &tcp_client_len)) == -1) {
                perror("TCP accept error");
                continue;
            }

            printf("TCP client connected: %s\n", inet_ntoa(tcp_client_addr.sin_addr));
            handle_tcp_connection(tcp_client_socket, motd);
        }

        // Handle UDP connections
        if (FD_ISSET(udp_server_socket, &read_fds)) {
            handle_udp_connection(udp_server_socket, motd);
        }
    }

    close(tcp_server_socket);
    close(udp_server_socket);

    return 0;
}
