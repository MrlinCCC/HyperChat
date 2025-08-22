add_executable(Benchmark RunBenchmark.cpp BenchmarkLog.hpp BenchmarkSerializer.hpp)

find_package(benchmark REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

target_link_libraries(Benchmark PRIVATE benchmark::benchmark Common jsoncpp_lib)