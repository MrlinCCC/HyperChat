add_executable(Benchmark RunBenchmark.cpp BenchmarkLog.hpp)
add_executable(LogTest LogTest.cpp)

find_package(benchmark REQUIRED)

target_link_libraries(Benchmark PRIVATE benchmark::benchmark)
target_link_libraries(Benchmark PRIVATE Logger)
target_link_libraries(LogTest PRIVATE Logger)