add_library(Common STATIC 
    src/Logger.cpp 
    src/ThreadPool.cpp 
    src/Protocol.cpp
    src/Semaphore.cpp
    src/Timestamp.cpp
    src/Spinlock.cpp
    src/Protocol.cpp
    src/Connection.cpp
    src/Timer.cpp
    )

find_package(OpenSSL CONFIG REQUIRED)

target_include_directories(Common PUBLIC include)
target_link_libraries(Common PUBLIC fmt::fmt )
target_link_libraries(Common PRIVATE OpenSSL::SSL OpenSSL::Crypto)