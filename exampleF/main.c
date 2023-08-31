#include <stdio.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "yahdlc/yahdlc.h"

#include "protocol.pb.h"

#include "ring_buffer.h"

/*
Example of samples:
[{"channel":"channel_2000_200_1","frequency":200,"time":0.051526546478271484,"value":0.31812476943300294}]


[{"channel":"channel_200_20_1","frequency":200,"time":0.051526546478271484,"value":0.31812476943300294},
{"channel":"channel_200_20_1","frequency":200,"time":0.056981563568115234,"value":0.3504258283522246},
{"channel":"channel_200_20_1","frequency":200,"time":0.06265902519226074,"value":0.3836063675227343},
{"channel":"channel_200_20_1","frequency":200,"time":0.06832265853881836,"value":0.4162198073016087}]
*/

#define BINARY_DATA_BUFFER_SIZE 256
#define WRITE_TO_FILE

#define SAMPLES_TO_ENCODE_BUFFER_SIZE (5 * sizeof(Batch_Sample))
#define DECODED_SAMPLES_BUFFER_SIZE (5 * sizeof(Batch_Sample))

#define HDLC_FRAME_BUFFER_SIZE (BINARY_DATA_BUFFER_SIZE + 64)

/* clang-format off */
static bool custom_repeated_encoding_callback(pb_ostream_t *stream, const pb_field_iter_t *field, void *const *arg);

static bool custom_repeated_decoding_callback(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);
/* clang-format on */


// RB containing raw samples that need to be encoded
static uint8_t    samples_to_encode_buffer[SAMPLES_TO_ENCODE_BUFFER_SIZE];
static RINGBUFF_T samples_to_encode_rb;
// RB containing raw samples were already decoded
static uint8_t    decoded_samples_buffer[DECODED_SAMPLES_BUFFER_SIZE];
static RINGBUFF_T decoded_samples_rb;

// Buffer to encode payload as protobuf
static uint8_t protobuf_binary_payload_buffer[BINARY_DATA_BUFFER_SIZE];

// Buffer to contain HDLC frame
static uint8_t      e_hdlc_frame_buffer[HDLC_FRAME_BUFFER_SIZE];
static unsigned int e_hdlc_frame_len;

// Buffer to simulate UART ISR
static uint8_t  uart_isr_buffer[BINARY_DATA_BUFFER_SIZE];
static uint16_t uart_isr_buffer_idx;

// Buffer to place the recovered protobuf binary payload from the HDLC frame
static uint8_t      binary_payload_recovered_from_hdlc_buffer[BINARY_DATA_BUFFER_SIZE];
static unsigned int binary_payload_recovered_len;

int main()
{
    RingBuffer_Init(&samples_to_encode_rb, samples_to_encode_buffer, sizeof(Batch_Sample),
                    DECODED_SAMPLES_BUFFER_SIZE);
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

    pb_ostream_t oStream =
        pb_ostream_from_buffer(protobuf_binary_payload_buffer, BINARY_DATA_BUFFER_SIZE);

    Batch batch_e = {.items.arg          = &samples_to_encode_rb,
                     .items.funcs.encode = &custom_repeated_encoding_callback};

    if (!pb_encode(&oStream, Batch_fields, &batch_e))
    {
        const char *error = PB_GET_ERROR(&oStream);
        printf("pb_encode error: %s\n", error);
        return 1;
    }
    size_t total_bytes_encoded = oStream.bytes_written;
    printf("Encoded size: %ld\n", total_bytes_encoded);

    yahdlc_control_t control_send;
    control_send.frame = YAHDLC_FRAME_DATA;
    int rc1 = yahdlc_frame_data(&control_send, protobuf_binary_payload_buffer, total_bytes_encoded,
                                e_hdlc_frame_buffer, &e_hdlc_frame_len);
    if (rc1)
    {
        printf("yahdlc_frame_data error: %d\n", rc1);
        return 2;
    }
    printf("HDLC frame size: %d\n", e_hdlc_frame_len);

#ifdef WRITE_TO_FILE

    FILE  *file_b_proto = fopen("protobuf_binary.bin", "wb");
    size_t foo =
        fwrite(protobuf_binary_payload_buffer, sizeof(uint8_t), total_bytes_encoded, file_b_proto);
    fclose(file_b_proto);

    FILE *file_h_proto = fopen("protobuf_hexa.hex", "wb");
    for (size_t i = 0; i < total_bytes_encoded; i++)
    {
        fprintf(file_h_proto, "%02x", protobuf_binary_payload_buffer[i]);
    }
    fclose(file_h_proto);

    FILE  *file_b_hdlc = fopen("hdlc_binary_frame.bin", "wb");
    size_t bar = fwrite(e_hdlc_frame_buffer, sizeof(uint8_t), e_hdlc_frame_len, file_b_hdlc);
    fclose(file_b_hdlc);

    FILE *file_h_hdlc = fopen("hdlc_hexa_frame.hex", "wb");
    for (size_t i = 0; i < e_hdlc_frame_len; i++)
    {
        fprintf(file_h_hdlc, "%02x", e_hdlc_frame_buffer[i]);
    }
    fclose(file_h_hdlc);
#endif

    printf("--------------------------------------------------------------------\n");

    yahdlc_control_t control_recv;
    for (uint32_t k = 0; k < e_hdlc_frame_len; k++)
    {
        /* Insert one byte at a time in an intermediary buffer to simulate an UART ISR */
        uart_isr_buffer[k] = e_hdlc_frame_buffer[k];
        uart_isr_buffer_idx++;

        // Get the data from the frame
        int rc2 = yahdlc_get_data(&control_recv, uart_isr_buffer, uart_isr_buffer_idx,
                                  binary_payload_recovered_from_hdlc_buffer,
                                  &binary_payload_recovered_len);

        // Success -> complete HDLC frame found -> will decode the binary payload now
        if (rc2 >= 0)
        {
            pb_istream_t iStream = pb_istream_from_buffer(binary_payload_recovered_from_hdlc_buffer,
                                                          binary_payload_recovered_len);

            /* Allocate space for the decoded message. */
            Batch batch_d = {.items.arg          = &decoded_samples_rb,
                             .items.funcs.decode = &custom_repeated_decoding_callback};

            /* Now we are ready to decode the message. */
            if (!pb_decode(&iStream, Batch_fields, &batch_d))
            {
                printf("Decoding failed: %s\n", PB_GET_ERROR(&iStream));
                return 3;
            }

            while (RingBuffer_GetCount(&decoded_samples_rb))
            {
                Batch_Sample sample_d = {};
                RingBuffer_Pop(&decoded_samples_rb, &sample_d);
                printf("~~~~~~~~~~~~~~~\n");
                printf("sample_d.name: %s\n", sample_d.name);
                printf("sample_d.frequency: %d\n", sample_d.frequency);
                printf("sample_d.time: %f\n", sample_d.time);
                printf("sample_d.value: %f\n", sample_d.value);
                printf("~~~~~~~~~~~~~~~\n");
            }
        }
    }
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