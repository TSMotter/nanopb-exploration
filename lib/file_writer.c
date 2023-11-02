#include "file_writer.h"

void write_binary_file(const char *filename, uint8_t *buffer, size_t buffer_len)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        perror("Error opening file for writing");
        return;
    }
    size_t bytes_written = fwrite(buffer, sizeof(uint8_t), buffer_len, file);
    if (bytes_written != buffer_len)
    {
        perror("Error writing to file");
    }
    fclose(file);
}

void write_hex_file(const char *filename, uint8_t *buffer, size_t buffer_len)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        perror("Error opening file for writing");
        return;
    }
    for (size_t i = 0; i < buffer_len; i++)
    {
        fprintf(file, "%02x", buffer[i]);
    }
    fclose(file);
}
