#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define CONFIG_FILE "config.conf"
#define TCP_SERVER_PORT 25565
#define UDP_SERVER_PORT 19132

void tcp_connect(const char *tcp_server_ip, const char *motd) {
    int tcp_sock;
    struct sockaddr_in tcp_server_addr;

    // Create TCP socket
    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("TCP socket creation error");
        exit(EXIT_FAILURE);
    }

    // Set TCP server address parameters
    tcp_server_addr.sin_family = AF_INET;
    tcp_server_addr.sin_port = htons(TCP_SERVER_PORT);
    
    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, tcp_server_ip, &tcp_server_addr.sin_addr) <= 0) {
        perror("Invalid TCP address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to TCP server
    if (connect(tcp_sock, (struct sockaddr *)&tcp_server_addr, sizeof(tcp_server_addr)) < 0) {
        perror("TCP connection failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to TCP server.\n");

    // Example: Send a TCP handshake packet
    char tcp_handshake_packet[1024];
    sprintf(tcp_handshake_packet, "\x00\x00%s", motd); // TCP handshake packet content with motd
    send(tcp_sock, tcp_handshake_packet, strlen(tcp_handshake_packet), 0);
    
    // Further TCP communication can be implemented here
    close(tcp_sock);
}

void udp_connect(const char *udp_server_ip, const char *motd) {
    int udp_sock;
    struct sockaddr_in udp_server_addr;

    // Create UDP socket
    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP socket creation error");
        exit(EXIT_FAILURE);
    }

    // Set UDP server address parameters
    udp_server_addr.sin_family = AF_INET;
    udp_server_addr.sin_port = htons(UDP_SERVER_PORT);
    
    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, udp_server_ip, &udp_server_addr.sin_addr) <= 0) {
        perror("Invalid UDP address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Example: Send a UDP ping packet
    char udp_ping_packet[1024];
    sprintf(udp_ping_packet, "\x00\x00%s", motd); // UDP ping packet content with motd
    sendto(udp_sock, udp_ping_packet, strlen(udp_ping_packet), 0, (const struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr));
    
    printf("Sent UDP ping packet.\n");

    // Further UDP communication can be implemented here
    close(udp_sock);
}

int main() {
    FILE *config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL) {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }

    char tcp_server_ip[100], udp_server_ip[100], motd[256];
    while (fscanf(config_file, "%*[^=]=%99s", tcp_server_ip) != EOF) {
        fscanf(config_file, "%*[^=]=%99s", udp_server_ip);
        fscanf(config_file, "%*[^=]=%255[^\n]", motd);
    }

    fclose(config_file);

    tcp_connect(tcp_server_ip, motd);
    udp_connect(udp_server_ip, motd);
    return 0;
}
