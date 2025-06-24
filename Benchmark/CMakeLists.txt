add_executable(Benchmark RunBenchmark.cpp BenchmarkLog.hpp)

find_package(benchmark REQUIRED)

target_link_libraries(Benchmark PRIVATE benchmark::benchmark)
target_link_libraries(Benchmark PRIVATE Common)