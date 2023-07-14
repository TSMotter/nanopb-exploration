#include <stdio.h>

#include "protocol.pb.h"
#include "ring_buffer.h"

#define RB_LEN 512

static RINGBUFF_T rb_encode, rb_decode;
static uint8_t rb_encode_buffer[RB_LEN] = {0}, rb_decode_buffer[RB_LEN] = {0};
uint8_t common_buffer[128];


int main()
{
    RingBuffer_Init(&rb_encode, rb_encode_buffer, sizeof(uint8_t), RB_LEN);
    RingBuffer_Init(&rb_decode, rb_decode_buffer, sizeof(uint8_t), RB_LEN);

    Sample sample_e = Sample_init_zero;

    printf("Execution will start... Press Enter to exit\n");
}
