#include "../include/utils.h"
#include "../include/protocol.h"
#include "../include/sha256.h"
#include <signal.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <libgen.h>
int server_socket;
void handle_sigint(int sig) {
    printf("\nПолучен сигнал завершения. Освобождаю порт и выключаю сервер...\n");
    close(server_socket);
    exit(0); // Штатно завершаем программу
}
int receive_file(int server_socket, FILE *file, struct sockaddr_in *client_addr, socklen_t *addr_size){
    Pack packet;
    int count = 0;
    while(1){
        *addr_size = sizeof(*client_addr);
        int ret = recvfrom(server_socket, &packet, sizeof(Pack), 0, (struct sockaddr *)client_addr, addr_size);
        if(ret<0){
            perror("recvfrom packet error");
            return -1;
        }
        if(packet.data_size == 0){
            printf("Получен маркер окончания файла\n");
            if(count != packet.current_number){
                printf("Файл получен не полностью. (получено %d из %d)\n", count, packet.current_number);
                return -1;
            }
            else{
                return 0;
            }
        }
        long offset = (long)packet.current_number*BUFFER_SIZE;
        if(fseek(file, offset, SEEK_SET)!= 0){
            perror("fseek error");
            continue;
        }
        fwrite(packet.data, 1, packet.data_size, file);
        count += 1;
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
    int ret;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    if (argc != 2){
        printf("%s <port-number>", argv[0]);
        return -1;
    }

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(server_socket < 0){
        perror("Socketfailed");
        return -2;
    }
    
    signal(SIGINT, handle_sigint);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    validate_convert_port(argv[1], &server_addr);

    ret = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret < 0){
        perror("Bind failed");
        (void)close(server_socket);
        return -3;
    }
    printf("Сервер запущен на порту: %s, и ожидает запросы...\n", argv[1]);

    while(1){
        addr_size = sizeof(client_addr);
        ret = recvfrom(server_socket, buffer, BUFFER_SIZE-1, 0, (struct sockaddr *)&client_addr, &addr_size);
        if(ret<0){
            perror("recvfrom error");
            continue;
        }
        buffer[ret] = '\0';
        char *clean_filename = basename(buffer);
        if(strcmp(clean_filename, ".")==0 || strcmp(clean_filename, "..")==0 || strlen(clean_filename) == 0){
            clean_filename = "unnamed_file";
        }
        char output_filename[BUFFER_SIZE+16];
        snprintf(output_filename, sizeof(output_filename), "recv_%s", clean_filename);
        printf("\nИнициирован прием файла: %s (сохраняем как %s)", buffer, output_filename);

        FILE *file = fopen(output_filename, "wb+");
        if(file == NULL){
            perror("Ошибка создания файла на сервере");
            continue;
        }
        
        if(receive_file(server_socket, file, &client_addr, &addr_size) < 0){
            printf("[Gebrochen] Прием прерван: пакеты утеряны. Файл удален.\n");
            fclose(file);
            remove(output_filename);
            char dummy[SHA256_HEX_SIZE];
            addr_size = sizeof(client_addr);
            recvfrom(server_socket, dummy, sizeof(dummy) - 1, MSG_DONTWAIT, (struct sockaddr *)&client_addr, &addr_size);

            continue;
        }
        fflush(file);
        char local_hash[SHA256_HEX_SIZE];
        int fd = fileno(file);
        hash_file_mmap(fd, local_hash);
        fclose(file);

        char remote_hash[SHA256_HEX_SIZE];
        addr_size = sizeof(client_addr);
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
            printf("[Gebrochen] Несовпадение хэшей! Пакеты были утеряны или повреждены! Файл не сохранен");
            remove(output_filename);
        }
    }
    (void)close(server_socket);
    return 0;
}
