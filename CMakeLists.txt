cmake_minimum_required(VERSION 3.20)
project(filter LANGUAGES CUDA CXX)

file(GLOB_RECURSE SRC src/*.cu src/*.cc)

add_executable(filter)
set_target_properties(filter PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
)
target_compile_features(filter PUBLIC cuda_std_17 cxx_std_17)
target_sources(filter PRIVATE ${SRC})
target_include_directories(filter PRIVATE inc)
target_compile_options(filter PRIVATE -Og -ggdb)
target_link_libraries(filter PRIVATE netfilter_queue)

target_compile_definitions(filter PRIVATE DEBUG)
