# ******************************************************************************
# **** Creates binary targets ****
# ******************************************************************************
# In this case, libraries
add_library(pbh INTERFACE pb.h)
add_library(pb_common
            pb_common.c
            pb_common.h)
add_library(pb_encode
            pb_encode.c
            pb_encode.h)
add_library(pb_decode
            pb_decode.c
            pb_decode.h)

# ******************************************************************************
# **** Link the libraries ****
# ******************************************************************************
target_link_libraries(pb_common PRIVATE
                      pbh)
target_link_libraries(pb_encode PRIVATE
                      pbh
                      pb_common)
target_link_libraries(pb_decode PRIVATE
                      pbh
                      pb_common)