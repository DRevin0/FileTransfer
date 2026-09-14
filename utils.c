#include "utils.h"

// Передаем указатель на порт который ввел пользователь в консоль | Указатель на структуру адреса, куда необходимо записать сам порт
void validate_convert_port(char *port_str, struct sockaddr_in *sock_addr){

    //Если порт или адрес равен 0 вывод ошибки через perror и завершение программы
    int port;
    if (port_str == NULL){
        perror("Invalid port_str\n");
        exit(EXIT_FAILURE);
    }
    if (sock_addr == NULL){
        perror("Invalid sock_addr\n");
        exit(EXIT_FAILURE);
    }
    // atoi приводит строку к числу, если были буквы в порте, то вывод ошибки
    port = atoi(port_str);
    if (port == 0){
        perror("Invalid prot\n");
        exit(EXIT_FAILURE);
    }
    
    // Берем поле sin_port и записываем в него перевернутое значение(с помощью htons), так как в пк и в сети данные храняться по разному
    sock_addr->sin_port = htons((uint16_t)port);
    // Чисто проверка
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