/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.5 */

#ifndef PB_EXAMPLEA_PROTOCOL_PB_H_INCLUDED
#define PB_EXAMPLEA_PROTOCOL_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _Sample
{
    uint32_t frequency;
    double   time;
    double   value;
} Sample;


#ifdef __cplusplus
extern "C"
{
#endif

/* Initializer values for message structs */
#define Sample_init_default \
    {                       \
        0, 0, 0             \
    }
#define Sample_init_zero \
    {                    \
        0, 0, 0          \
    }

/* Field tags (for use in manual encoding/decoding) */
#define Sample_frequency_tag 1
#define Sample_time_tag 2
#define Sample_value_tag 3

/* Struct field encoding specification for nanopb */
#define Sample_FIELDLIST(X, a)                   \
    X(a, STATIC, SINGULAR, UINT32, frequency, 1) \
    X(a, STATIC, SINGULAR, DOUBLE, time, 2)      \
    X(a, STATIC, SINGULAR, DOUBLE, value, 3)
#define Sample_CALLBACK NULL
#define Sample_DEFAULT NULL

    extern const pb_msgdesc_t Sample_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define Sample_fields &Sample_msg

/* Maximum encoded size of messages (where known) */
#define Sample_size 24

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
