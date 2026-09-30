#include "../include/utils.h"

void validate_convert_port(char *port_str, struct sockaddr_in *sock_addr){

    int port;
    if (port_str == NULL){
        perror("Invalid port_str\n");
        exit(EXIT_FAILURE);
    }
    if (sock_addr == NULL){
        perror("Invalid sock_addr\n");
        exit(EXIT_FAILURE);
    }

    port = atoi(port_str);
    if (port == 0){
        perror("Invalid prot\n");
        exit(EXIT_FAILURE);
    }

    sock_addr->sin_port = htons((uint16_t)port);
    printf("Port: %d\n", ntohs(sock_addr->sin_port));
}

void validate_convert_addr(char *ip_str, struct sockaddr_in *sock_addr){
    if(ip_str==NULL){
        perror("Ivalid ip_str\n");
        exit(EXIT_FAILURE);
    }
    if (sock_addr == NULL){
        perror("Invalid sock_addr\n");
        exit(EXIT_FAILURE);
    }

    printf("IP Address: %s\n", ip_str);
    if(inet_pton(AF_INET, ip_str, &(sock_addr->sin_addr))<=0){
        perror("Ivalid address\n");
        exit(EXIT_FAILURE);
    }
}