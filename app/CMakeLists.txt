# Add a cmake binary taget (in this case, an executable)
add_executable(main main.c)

# Add a cmake binary taget (in this case, a library)
add_library(protocolpb protocol.pb.c protocol.pb.h)
add_library(ring_buffer ring_buffer.c ring_buffer.h)

# Link library to a binary target
target_link_libraries(main PRIVATE
    protocolpb
    ring_buffer
)