#include "../include/utils.h"
#include "../include/protocol.h"
#include "../include/sha256.h"
#include <signal.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
// argc - количество аргументов при запуске, argv - массив аргументов. Пр: ./server 8080 - argc == 2; argv[0] = ./server;argv[1]=8080
int server_socket;
void handle_sigint(int sig) {
    printf("\nПолучен сигнал завершения. Освобождаю порт и выключаю сервер...\n");
    close(server_socket);
    exit(0); // Штатно завершаем программу
}
void receive_file(int server_socket, FILE *file, struct sockaddr_in *client_addr, socklen_t *addr_size){
    Pack packet;
    while(1){
        int ret = recvfrom(server_socket, &packet, sizeof(Pack), 0, (struct sockaddr *)client_addr, addr_size);
        if(ret<0){
            perror("recvfrom packet error");
            break;
        }
        if(packet.data_size == 0){
            printf("Получен маркер окончания файла\n");
            break;
        }
        long offset = (long)packet.current_number*BUFFER_SIZE;
        if(fseek(file, offset, SEEK_SET)!= 0){
            perror("fseek error");
            continue;
        }
        fwrite(packet.data, 1, packet.data_size, file);
        printf("Записан пакет %d (смещение: %ld, байт: %zu\n)\n", packet.current_number, offset, packet.data_size);
    }
}
void hash_file_mmap(int fd, char *out_hash){
    struct stat st;
    if(fstat(fd, &st)<0){
        perror("fstat failed");
        return;
    }
    size_t file_size = st.st_size;
    if(file_size == 0){
        sha256 sha;
        sha256_init(&sha);
        sha256_finalize_hex(&sha, out_hash);
        return;
    }
    char *mapped = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
        if(mapped == MAP_FAILED){
            perror("mmap failed");
            return;
        }
        madvise(mapped, file_size, MADV_SEQUENTIAL);
        sha256 sha;
        sha256_init(&sha);
        sha256_append(&sha, mapped, file_size);
        sha256_finalize_hex(&sha, out_hash);

        munmap(mapped, file_size);
}

int main(int argc, char *argv[]){
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
    
    signal(SIGINT, handle_sigint);

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
    printf("Сервер запущен на порту: %s, и ожидает запросы...\n", argv[1]);

    while(1){
        ret = recvfrom(server_socket, buffer, BUFFER_SIZE-1, 0, (struct sockaddr *)&client_addr, &addr_size);
        if(ret<0){
            perror("recvfrom error");
            continue;
        }
        buffer[ret] = '\0';
        char output_filename[BUFFER_SIZE+10];
        snprintf(output_filename, sizeof(output_filename), "recv_%s", buffer);
        printf("\nИнициирован прием файла: %s (сохраняем как %s)", buffer, output_filename);

        FILE *file = fopen(output_filename, "wb+");
        if(file == NULL){
            perror("Ошибка создания файла на сервере");
            continue;
        }
        
        receive_file(server_socket, file, &client_addr, &addr_size);
        fflush(file);
        char local_hash[SHA256_HEX_SIZE];
        int fd = fileno(file);
        hash_file_mmap(fd, local_hash);
        fclose(file);

        char remote_hash[SHA256_HEX_SIZE];
        ret = recvfrom(server_socket, remote_hash, sizeof(remote_hash)-1, 0, (struct sockaddr *)&client_addr,&addr_size);
        if(ret<0){
            perror("recvfrom hash error");
            continue;
        }
        remote_hash[ret] = '\0';
        printf("  Сервер (SHA-256): %s\n", local_hash);
        printf("  Клиент (SHA-256): %s\n", remote_hash);

        if(strcmp(local_hash, remote_hash)==0){
            printf("[Alles ist gut] Файл доставлен без потерь и поверждений\n");
        }
        else{
            printf("[Gebrochen] Несовпадение хэшей! Пакеты были утеряны или повреждены!");
        }
    }
    (void)close(server_socket);
    return 0;
}
