#include <stdio.h>

#include "pb_encode.h"
#include "pb_decode.h"

#include "protocol.pb.h"

#include "ring_buffer/ring_buffer.h"

/*
Example of samples:
[{"channel":"channel_2000_200_1","frequency":200,"time":0.051526546478271484,"value":0.31812476943300294}]


[{"channel":"channel_200_20_1","frequency":200,"time":0.051526546478271484,"value":0.31812476943300294},
{"channel":"channel_200_20_1","frequency":200,"time":0.056981563568115234,"value":0.3504258283522246},
{"channel":"channel_200_20_1","frequency":200,"time":0.06265902519226074,"value":0.3836063675227343},
{"channel":"channel_200_20_1","frequency":200,"time":0.06832265853881836,"value":0.4162198073016087}]
*/

#define COMMON_BUFFER_SIZE 512
#define WRITE_TO_FILE

/* clang-format off */
static bool custom_ostream_callback(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);
static bool custom_field_encoding_callback(pb_ostream_t *stream, const pb_field_iter_t *field, void *const *arg);
static bool custom_repeated_encoding_callback(pb_ostream_t *stream, const pb_field_iter_t *field, void *const *arg);

static bool custom_istream_callback(pb_istream_t *stream, pb_byte_t *buf, size_t count);
static bool custom_field_decoding_callback(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);
static bool custom_repeated_decoding_callback(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);
/* clang-format on */

static uint8_t    common_buffer[COMMON_BUFFER_SIZE];
static RINGBUFF_T common_rb;


int main()
{
    RingBuffer_Init(&common_rb, common_buffer, sizeof(uint8_t), COMMON_BUFFER_SIZE);

    pb_ostream_t oStream = {&custom_ostream_callback, (void *) &common_rb, COMMON_BUFFER_SIZE, 0};

    Batch_Sample samples[2] = {{.name.arg          = "channel_2000_200_1",
                                .name.funcs.encode = &custom_field_encoding_callback,
                                .frequency         = 200,
                                .time              = 0.051526546478271484,
                                .value             = 0.31812476943300294},
                               {.name.arg          = "channel_2000_200_1",
                                .name.funcs.encode = &custom_field_encoding_callback,
                                .frequency         = 200,
                                .time              = 0.056981563568115234,
                                .value             = 0.3504258283522246}};

    Batch batch_e = {.items.arg          = samples,
                     .items.funcs.encode = &custom_repeated_encoding_callback};


    if (!pb_encode(&oStream, Batch_fields, &batch_e))
    {
        const char *error = PB_GET_ERROR(&oStream);
        printf("pb_encode error: %s\n", error);
    }
    size_t total_bytes_encoded = oStream.bytes_written;
    printf("Encoded size: %ld\n", total_bytes_encoded);

#ifdef WRITE_TO_FILE
    FILE  *fileb            = fopen("binary.bin", "wb");
    size_t elements_written = fwrite(common_buffer, sizeof(uint8_t), total_bytes_encoded, fileb);
    fclose(fileb);
    FILE *fileh = fopen("hexa.hex", "wb");
    for (size_t i = 0; i < total_bytes_encoded; i++)
    {
        fprintf(fileh, "%02x", common_buffer[i]);
    }
    fclose(fileh);
#endif

    printf("--------------------------------------------------------------------\n");

    pb_istream_t iStream = {&custom_istream_callback, (void *) &common_rb, COMMON_BUFFER_SIZE, 0};

    /* Allocate space for the decoded message. */
    Batch batch_d = {.items.arg = NULL, .items.funcs.decode = &custom_repeated_decoding_callback};

    /* Now we are ready to decode the message. */
    if (!pb_decode(&iStream, Batch_fields, &batch_d))
    {
        printf("Decoding failed: %s\n", PB_GET_ERROR(&iStream));
        return 1;
    }
}


static bool custom_ostream_callback(pb_ostream_t *stream, const pb_byte_t *buf, size_t count)
{
    RINGBUFF_T *rb = (RINGBUFF_T *) stream->state;

    printf("custom_ostream_callback!-> count:%ld\n", count);

    return RingBuffer_InsertMult(rb, buf, count) == count;
}

static bool custom_field_encoding_callback(pb_ostream_t *stream, const pb_field_iter_t *field,
                                           void *const *arg)
{
    printf("custom_field_encoding_callback!-> tag:%d, (char *) *arg:%s\n", field->tag,
           (char *) *arg);


    char *raw_string = (char *) *arg;

    if (!pb_encode_tag_for_field(stream, field))
    {
        return false;
    }

    return pb_encode_string(stream, (uint8_t *) raw_string, strlen(raw_string));
}

static bool custom_repeated_encoding_callback(pb_ostream_t *stream, const pb_field_iter_t *field,
                                              void *const *arg)
{
    printf("custom_repeated_encoding_callback!-> tag:%d\n", field->tag);

    Batch_Sample *samples = (Batch_Sample *) *arg;

    for (uint16_t k = 0; k < 2; k++)
    {
        if (!pb_encode_tag_for_field(stream, field))
        {
            const char *error = PB_GET_ERROR(stream);
            printf("pb_encode error: %s\n", error);
            return false;
        }

        if (!pb_encode_submessage(stream, Batch_Sample_fields, &samples[k]))
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

    printf("custom_istream_callback!-> count:%ld, *buf:%02x\n", count, *buf);

    if (RingBuffer_IsEmpty(rb))
    {
        printf("RingBuffer_IsEmpty\n");
        stream->bytes_left = 0;
        return false;
    }

    return RingBuffer_PopMult(rb, buf, count) == count;
}

static bool custom_field_decoding_callback(pb_istream_t *stream, const pb_field_iter_t *field,
                                           void **arg)
{
    printf("custom_field_decoding_callback!-> tag:%d, (char *) *arg:%s, stream->bytes_left:%ld\n",
           field->tag, (char *) *arg, stream->bytes_left);

    char *dest = (char *) *arg;

    while (stream->bytes_left)
    {
        uint64_t decoded_var;
        if (!pb_decode_varint(stream, &decoded_var))
        {
            return false;
        }
        strcat(dest, (char *) &decoded_var);
    }
    return true;
}

static bool custom_repeated_decoding_callback(pb_istream_t *stream, const pb_field_iter_t *field,
                                              void **arg)
{
    printf("custom_repeated_decoding_callback!-> tag:%d\n", field->tag);

    if (stream != NULL && field->tag == Batch_items_tag)
    {
        char         sample_d_name[25] = "";
        Batch_Sample sample_d          = {.name.arg          = sample_d_name,
                                          .name.funcs.decode = &custom_field_decoding_callback};

        if (!pb_decode(stream, Batch_Sample_fields, &sample_d))
        {
            printf("Decoding failed: %s\n", PB_GET_ERROR(stream));
            return 1;
        }

        printf("sample_d.name: %s\n", sample_d_name);
        printf("sample_d.frequency: %d\n", sample_d.frequency);
        printf("sample_d.time: %f\n", sample_d.time);
        printf("sample_d.value: %f\n", sample_d.value);
    }

    return true;
}