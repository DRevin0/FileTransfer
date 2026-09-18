#include "../include/utils.h"
#include "../include/protocol.h"
#include "../include/sha256.h"
#include <libgen.h>
void dropfile(char *a, int client_socket, struct sockaddr_in *server_addr){
    FILE *file = fopen(a, "rb");
    if(file == NULL){
        perror("Error in fopen\n");
        exit(-777);
    }
    sha256 sha;
    sha256_init(&sha);

    Pack packet;
    packet.current_number = 0;
    while((packet.data_size = fread(packet.data, 1, BUFFER_SIZE, file))>0){
        sha256_append(&sha, packet.data, packet.data_size);
        sendto(client_socket, &packet, sizeof(Pack), 0, (struct sockaddr*)server_addr, sizeof(*server_addr));
        packet.current_number += 1;
    }
    fclose(file);
    char hex_hash[SHA256_HEX_SIZE];
    sha256_finalize_hex(&sha, hex_hash);
    printf("SHA-256 файла: %s\n", hex_hash);
    packet.data_size = 0;
    sendto(client_socket, &packet, sizeof(Pack), 0, (struct sockaddr *)server_addr, sizeof(*server_addr));//Отправка последнего пакета с 0
    sendto(client_socket, hex_hash, strlen(hex_hash), 0, (struct sockaddr *)server_addr, sizeof(*server_addr));//Отправка хэша
    
}

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
    char *clean_name = basename(filename);
    snprintf(buffer, BUFFER_SIZE, "%s", clean_name);
    ret = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret<0){
        perror("sendto");
        (void)close(client_socket);
        return -3;
    }
    printf("Инициирована отправка файла: %s\n", buffer);
    dropfile(filename, client_socket, &server_addr);
    (void)close(client_socket);
    return 0;
}