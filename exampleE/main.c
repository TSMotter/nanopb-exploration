#include <stdio.h>

#include "protocol.pb.h"
#include "file_writer.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "ring_buffer.h"

/*
Example of samples:
[{"channel":"channel_2000_200_1","frequency":200,"time":0.051526546478271484,"value":0.31812476943300294}]


[{"channel":"channel_200_20_1","frequency":200,"time":0.051526546478271484,"value":0.31812476943300294},
{"channel":"channel_200_20_1","frequency":200,"time":0.056981563568115234,"value":0.3504258283522246},
{"channel":"channel_200_20_1","frequency":200,"time":0.06265902519226074,"value":0.3836063675227343},
{"channel":"channel_200_20_1","frequency":200,"time":0.06832265853881836,"value":0.4162198073016087}]
*/

#define BINARY_DATA_BUFFER_SIZE 512
#define SAMPLES_TO_ENCODE_BUFFER_SIZE (5 * sizeof(Batch_Sample))
#define DECODED_SAMPLES_BUFFER_SIZE (5 * sizeof(Batch_Sample))

/* clang-format off */
static bool custom_ostream_callback(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);
static bool custom_repeated_encoding_callback(pb_ostream_t *stream, const pb_field_iter_t *field, void *const *arg);
static bool custom_istream_callback(pb_istream_t *stream, pb_byte_t *buf, size_t count);
static bool custom_repeated_decoding_callback(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);
/* clang-format on */

// RB containing raw samples that need to be encoded
static uint8_t    samples_to_encode_buffer[SAMPLES_TO_ENCODE_BUFFER_SIZE];
static RINGBUFF_T samples_to_encode_rb;

// RB containing bytes which represent the stream of encoded samples
static uint8_t    binary_data_buffer[BINARY_DATA_BUFFER_SIZE];
static RINGBUFF_T binary_data_rb;

// RB containing raw samples were already decoded
static uint8_t    decoded_samples_buffer[DECODED_SAMPLES_BUFFER_SIZE];
static RINGBUFF_T decoded_samples_rb;

int main()
{
    RingBuffer_Init(&samples_to_encode_rb, samples_to_encode_buffer, sizeof(Batch_Sample),
                    DECODED_SAMPLES_BUFFER_SIZE);
    RingBuffer_Init(&binary_data_rb, binary_data_buffer, sizeof(uint8_t), BINARY_DATA_BUFFER_SIZE);
    RingBuffer_Init(&decoded_samples_rb, decoded_samples_buffer, sizeof(Batch_Sample),
                    DECODED_SAMPLES_BUFFER_SIZE);

    Batch_Sample samples_e[2] = {{.name      = "channel_2000_200_1",
                                  .frequency = 200,
                                  .time      = 0.051526546478271484,
                                  .value     = 0.31812476943300294},
                                 {.name      = "channel_2000_200_1",
                                  .frequency = 200,
                                  .time      = 0.056981563568115234,
                                  .value     = 0.3504258283522246}};
    RingBuffer_InsertMult(&samples_to_encode_rb, samples_e,
                          (sizeof(samples_e) / sizeof(samples_e[0])));

    pb_ostream_t oStream = {&custom_ostream_callback, (void *) &binary_data_rb,
                            BINARY_DATA_BUFFER_SIZE, 0};

    Batch batch_e = {.items.arg          = &samples_to_encode_rb,
                     .items.funcs.encode = &custom_repeated_encoding_callback};

    if (!pb_encode(&oStream, Batch_fields, &batch_e))
    {
        const char *error = PB_GET_ERROR(&oStream);
        printf("pb_encode error: %s\n", error);
    }
    size_t total_bytes_encoded = oStream.bytes_written;
    printf("Encoded size: %ld\n", total_bytes_encoded);

    write_binary_file("protobuf_payload.bin", binary_data_buffer, total_bytes_encoded);
    write_hex_file("protobuf_payload.hex", binary_data_buffer, total_bytes_encoded);

    printf("--------------------------------------------------------------------\n");

    pb_istream_t iStream = {&custom_istream_callback, (void *) &binary_data_rb,
                            BINARY_DATA_BUFFER_SIZE, 0};

    /* Allocate space for the decoded message. */
    Batch batch_d = {.items.arg          = &decoded_samples_rb,
                     .items.funcs.decode = &custom_repeated_decoding_callback};

    /* Now we are ready to decode the message. */
    if (!pb_decode(&iStream, Batch_fields, &batch_d))
    {
        printf("Decoding failed: %s\n", PB_GET_ERROR(&iStream));
        return 1;
    }

    while (RingBuffer_GetCount(&decoded_samples_rb))
    {
        Batch_Sample sample_d = {};
        RingBuffer_Pop(&decoded_samples_rb, &sample_d);
        printf("~~~~Sample Recovered:~~~~\n");
        printf("sample_d.name: %s\n", sample_d.name);
        printf("sample_d.frequency: %d\n", sample_d.frequency);
        printf("sample_d.time: %f\n", sample_d.time);
        printf("sample_d.value: %f\n", sample_d.value);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    }
}

static bool custom_ostream_callback(pb_ostream_t *stream, const pb_byte_t *buf, size_t count)
{
    RINGBUFF_T *rb = (RINGBUFF_T *) stream->state;

    printf("custom_ostream_callback!-> count:%ld -> ", count);
    for (int k = 0; k < count; k++)
    {
        printf("0x%02x, ", buf[k]);
    }
    printf("\n");
    return RingBuffer_InsertMult(rb, buf, count) == count;
}

static bool custom_repeated_encoding_callback(pb_ostream_t *stream, const pb_field_iter_t *field,
                                              void *const *arg)
{
    printf("custom_repeated_encoding_callback!-> tag:%d\n", field->tag);

    // Batch_Sample *samples = (Batch_Sample *) *arg;
    RINGBUFF_T *rb = (RINGBUFF_T *) *arg;

    size_t num_samples = RingBuffer_GetCount(rb);
    for (uint16_t k = 0; k < num_samples; k++)
    {
        if (!pb_encode_tag_for_field(stream, field))
        {
            const char *error = PB_GET_ERROR(stream);
            printf("pb_encode error: %s\n", error);
            return false;
        }

        Batch_Sample s = {};
        RingBuffer_Pop(rb, &s);
        if (!pb_encode_submessage(stream, Batch_Sample_fields, &s))
        {
            const char *error = PB_GET_ERROR(stream);
            printf("pb_encode error: %s\n", error);
            return false;
        }
    }

    return true;
}

static bool custom_istream_callback(pb_istream_t *stream, pb_byte_t *buf, size_t count)
{
    RINGBUFF_T *rb = (RINGBUFF_T *) stream->state;

    printf("custom_istream_callback!-> count:%ld -> ", count);


    if (RingBuffer_IsEmpty(rb))
    {
        printf("RingBuffer_IsEmpty\n");
        stream->bytes_left = 0;
        return false;
    }

    bool status = RingBuffer_PopMult(rb, buf, count) == count;
    for (int k = 0; k < count; k++)
    {
        printf("0x%02x, ", buf[k]);
    }
    printf("\n");
    return status;
}

static bool custom_repeated_decoding_callback(pb_istream_t *stream, const pb_field_iter_t *field,
                                              void **arg)
{
    printf("custom_repeated_decoding_callback!-> tag:%d\n", field->tag);

    RINGBUFF_T *rb = (RINGBUFF_T *) *arg;

    if (stream != NULL && field->tag == Batch_items_tag)
    {
        Batch_Sample sample_d = Batch_Sample_init_zero;

        if (!pb_decode(stream, Batch_Sample_fields, &sample_d))
        {
            printf("Decoding failed: %s\n", PB_GET_ERROR(stream));
            return 1;
        }

        RingBuffer_Insert(rb, &sample_d);
    }

    return true;
}