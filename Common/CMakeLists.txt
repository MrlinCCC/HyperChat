add_library(Common STATIC 
    src/Logger.cpp 
    src/ThreadPool.cpp 
    src/Protocol.cpp
    src/Semaphore.cpp
    src/Timestamp.cpp
    src/Spinlock.cpp
    src/Protocol.cpp
    src/Connection.cpp
)

target_include_directories(Common PUBLIC include include/entity include/utils)
target_link_libraries(Common PUBLIC fmt::fmt)