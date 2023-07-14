#include <stdio.h>

#include "pb_encode.h"
#include "pb_decode.h"

#include "protocol.pb.h"
#include "ring_buffer.h"

#define RB_LEN 512

static bool custom_encode_cb(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);

static RINGBUFF_T rb_encode, rb_decode;
static uint8_t    rb_encode_buffer[RB_LEN] = {0}, rb_decode_buffer[RB_LEN] = {0};
uint8_t           common_buffer[128];

int main()
{
    RingBuffer_Init(&rb_encode, rb_encode_buffer, sizeof(uint8_t), RB_LEN);
    RingBuffer_Init(&rb_decode, rb_decode_buffer, sizeof(uint8_t), RB_LEN);

    Sample       sample_e = Sample_init_zero;
    pb_ostream_t oStream  = {&custom_encode_cb, (void *) &rb_encode, SIZE_MAX, 0};
    if (!pb_encode(&oStream, Sample_fields, &sample_e))
    {
        const char *error = PB_GET_ERROR(&oStream);
        printf("pb_encode error: %s\n", error);
    }
    size_t total_bytes_encoded = oStream.bytes_written;
    printf("Encoded size: %ld\n", total_bytes_encoded);

    printf("--------------------------------------------------------------------\n");
}

static bool custom_encode_cb(pb_ostream_t *stream, const pb_byte_t *buf, size_t count)
{
    RINGBUFF_T *fd = (RINGBUFF_T *) stream->state;

    printf("encode_cb!-> count:%ld, *buf:%02x\n", count, *buf);

    return RingBuffer_InsertMult(fd, buf, count) == count;
}