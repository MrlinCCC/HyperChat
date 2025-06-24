add_library(Common INTERFACE Logger.hpp ThreadPool.hpp)

target_include_directories(Common INTERFACE .)
target_link_libraries(Common INTERFACE fmt::fmt)