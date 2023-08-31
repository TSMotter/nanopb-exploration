# Add a cmake binary taget (in this case, an executable)
add_executable(main main.c)
add_library(protocolpb protocol.pb.c protocol.pb.h)

# Make the directory known
target_include_directories(main PUBLIC
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/lib
)

# Link library to a binary target
target_link_libraries(main PRIVATE
    protocolpb
    nanopb
    yahdlc
    ring_buffer
)