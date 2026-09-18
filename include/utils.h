#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>


void validate_convert_port(char *port_str, struct sockaddr_in *sock_addr);
void validate_convert_addr(char *ip_str, struct sockaddr_in *sock_addr);


#endif // UTILS_H