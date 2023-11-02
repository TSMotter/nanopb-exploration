#include <stdio.h>

#include "protocol.pb.h"
#include "file_writer.h"
#include "pb_encode.h"
#include "pb_decode.h"

/*
{"channel":"channel_200_20_1","frequency":200,"time":0.051526546478271484,"value":0.31812476943300294}
{"channel":"channel_200_20_1","frequency":200,"time":0.056981563568115234,"value":0.3504258283522246}
{"channel":"channel_200_20_1","frequency":200,"time":0.06265902519226074,"value":0.3836063675227343}
{"channel":"channel_200_20_1","frequency":200,"time":0.06832265853881836,"value":0.4162198073016087}
*/

#define COMMON_BUFFER_SIZE 512

uint8_t common_buffer[COMMON_BUFFER_SIZE];

int main()
{
    Sample sample_e = Sample_init_zero;

    pb_ostream_t oStream = pb_ostream_from_buffer(common_buffer, COMMON_BUFFER_SIZE);

    sample_e.frequency = 200;
    sample_e.time      = 0.051526546478271484;
    sample_e.value     = 0.31812476943300294;

    if (!pb_encode(&oStream, Sample_fields, &sample_e))
    {
        const char *error = PB_GET_ERROR(&oStream);
        printf("pb_encode error: %s\n", error);
    }
    size_t total_bytes_encoded = oStream.bytes_written;
    printf("Encoded size: %ld\n", total_bytes_encoded);

    write_binary_file("protobuf_payload.bin", common_buffer, total_bytes_encoded);
    write_hex_file("protobuf_payload.hex", common_buffer, total_bytes_encoded);

    printf("--------------------------------------------------------------------\n");

    /* Allocate space for the decoded message. */
    Sample sample_d = Sample_init_zero;

    /* Create a stream that reads from the buffer. */
    pb_istream_t iStream = pb_istream_from_buffer(common_buffer, total_bytes_encoded);

    /* Now we are ready to decode the message. */
    if (!pb_decode(&iStream, Sample_fields, &sample_d))
    {
        printf("Decoding failed: %s\n", PB_GET_ERROR(&iStream));
        return 1;
    }

    printf("~~~~Sample Recovered:~~~~\n");
    printf("sample_d.frequency: %d\n", sample_d.frequency);
    printf("sample_d.time: %f\n", sample_d.time);
    printf("sample_d.value: %f\n", sample_d.value);
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}
