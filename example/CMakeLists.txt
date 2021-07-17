add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PRIVATE lexertk::lexertk benchmark::benchmark benchmark::benchmark_main)

add_executable(example lexertk_examples.cpp)
target_link_libraries(example PRIVATE lexertk::lexertk)
