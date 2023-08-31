file(GLOB GLOB_NANOPB_LIB
    "*.h"
    "*.c"
)

add_library(nanopb ${GLOB_NANOPB_LIB})