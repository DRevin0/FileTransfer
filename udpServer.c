#include "utils.h"
// argc - количество аргументов при запуске, argv - массив аргументов. Пр: ./server 8080 - argc == 2; argv[0] = ./server;argv[1]=8080
int main(int argc, char *argv[]){
    int server_socket;//дескриптор
    int ret;// хранение ответов системных функций
    struct sockaddr_in server_addr, client_addr;//создаем готовые структуры(определены в системных библиотеках)
    socklen_t addr_size = sizeof(client_addr);//размер структуры адреса
    char buffer[BUFFER_SIZE];//коробка для приема сообщений

    if (argc != 2){
        printf("%s <port-number>", argv[0]);
        return -1;
    }

    //Создаем сокет. af_inet - ipv4, остальное для udp
    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(server_socket < 0){
        perror("Socketfailed");
        return -2;
    }

    server_addr.sin_family = AF_INET;// подтверждаем ipv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // слушаем запросы со всех сетевых карт
    validate_convert_port(argv[1], &server_addr);//Наша функция

    // связываем конкретные ip-адрес и порт с дескриптором, приводим типы для ipv4 вторым арг.
    ret = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret < 0){
        perror("Bind failed");
        (void)close(server_socket);
        return -3;
    }

    //аргументы: сокет;куда записывать данные;сколько байт данных можно записать;флаги = 0;адрес структуры клиента(ip и порт того кто пишет);и размер структуры.
    ret = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_size);
    if(ret < 0){
        perror("recvfrom");
        (void)close(server_socket);
        return -4;
    }
    printf("reveived: %s\n", buffer);
    (void)close(server_socket);
    return 0;
}
