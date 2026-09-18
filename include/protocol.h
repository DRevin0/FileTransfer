#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stddef.h>
#define BUFFER_SIZE 1024
typedef struct{
    int current_number;
    size_t data_size;
    char data[BUFFER_SIZE];
}Pack;
#endif