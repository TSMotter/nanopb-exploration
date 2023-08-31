file(GLOB GLOB_YAHDLC_LIB
    "*.h"
    "*.c"
)

add_library(yahdlc ${GLOB_YAHDLC_LIB})