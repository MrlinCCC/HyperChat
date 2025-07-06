add_library(Common STATIC 
    src/Logger.cpp 
    src/ThreadPool.cpp 
    src/ProtocolMessage.cpp
    src/Semaphore.cpp
    src/Timestamp.cpp
    src/Spinlock.cpp
)

target_include_directories(Common PUBLIC include)
target_link_libraries(Common PUBLIC fmt::fmt jsoncpp_lib)