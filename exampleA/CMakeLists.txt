# ******************************************************************************
# **** Creates binary targets ****
# ******************************************************************************
# In this case, an executable
add_executable(main main.c)
# In this case, a library
add_library(protocolpb
            protocol.pb.c
            protocol.pb.h)

# ******************************************************************************
# **** Make other directories known the targets ****
# ******************************************************************************
target_include_directories(main PRIVATE
                           ${CMAKE_SOURCE_DIR}/lib
                           ${CMAKE_SOURCE_DIR}/external/nanopb)
target_include_directories(protocolpb PRIVATE
                           ${CMAKE_SOURCE_DIR}/external/nanopb)

# ******************************************************************************
# **** Link the libraries ****
# ******************************************************************************
target_link_libraries(protocolpb PRIVATE
                      pbh)
target_link_libraries(main PRIVATE
                      file_writer
                      protocolpb
                      pb_encode
                      pb_decode)