
# Add a cmake binary taget (in this case, a library)
add_library(ring_buffer ring_buffer.c ring_buffer.h)

## Make the directory known
#target_include_directories(gm_lib PUBLIC
#    ${CMAKE_SOURCE_DIR}/external
#)
#
## Link library to a binary target
#target_link_libraries(gm_lib PRIVATE
#    lib1
#    lib2
#)