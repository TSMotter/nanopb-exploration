file(GLOB my_glob
    "*.h"
    "*.c"
)

add_library(nanopb ${my_glob})