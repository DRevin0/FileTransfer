#include "utils.h"
int main(int argc, char *argv[]){
    int client_socket;
    int ret;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = "Watermellon for everyone!";

    if(argc != 4){
        printf("%s<port-number><ip-addr><file-to-send>\n", argv[0]);
        return -1;
    }
    client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(client_socket<0){
        perror("Socket failed");
        return -2;
    }
    server_addr.sin_family = AF_INET;
    validate_convert_port(argv[1], &server_addr);
    validate_convert_addr(argv[2], &server_addr);

    char *filename = argv[3];
    snprintf(buffer, BUFFER_SIZE, "%s", filename);
    // TODO:
    ret = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret<0){
        perror("sendto");
        (void)close(client_socket);
        return -3;
    }
    printf("Инициирована отправка файла: %s\n", buffer);
    (void)close(client_socket);
    return 0;
}