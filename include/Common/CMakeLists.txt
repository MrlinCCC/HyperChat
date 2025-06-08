add_library(Logger INTERFACE)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(Logger INTERFACE fmt::fmt)

target_include_directories(Logger INTERFACE .)