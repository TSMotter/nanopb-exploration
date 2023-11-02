#ifndef FILE_WRITER_H_INCLUDED
#define FILE_WRITER_H_INCLUDED


#include <stdint.h>
#include <stdio.h>

void write_binary_file(const char *filename, uint8_t *buffer, size_t buffer_len);
void write_hex_file(const char *filename, uint8_t *buffer, size_t buffer_len);

#endif