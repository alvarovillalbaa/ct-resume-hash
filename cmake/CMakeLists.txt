cmake_minimum_required(VERSION 3.15)
project(ct_resume_hash C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(CT_RESUME_HASH_USE_CT "Use constant-time normalization implementation" ON)
option(CT_RESUME_HASH_BUILD_TESTS "Build unit tests" ON)
option(CT_RESUME_HASH_ENABLE_FUZZ "Build fuzz harnesses" ON)
option(CT_RESUME_HASH_ENABLE_BENCH "Build benchmarks" ON)

add_library(ct_resume_hash
    ${CMAKE_SOURCE_DIR}/src/ct_resume_hash.c
    ${CMAKE_SOURCE_DIR}/src/normalize_ref.c
    ${CMAKE_SOURCE_DIR}/src/normalize_ct.c
    ${CMAKE_SOURCE_DIR}/src/hash_core.c
    ${CMAKE_SOURCE_DIR}/src/sha256.c
)

target_include_directories(ct_resume_hash PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(ct_resume_hash PUBLIC
    $<$<BOOL:${CT_RESUME_HASH_USE_CT}>:CT_RESUME_HASH_USE_CT>
)

target_compile_options(ct_resume_hash PRIVATE
    -Wall -Wextra -Werror -pedantic
    -O2
    -fwrapv
    -fno-builtin-memcmp
)

enable_testing()

if(CT_RESUME_HASH_BUILD_TESTS)
    add_executable(test_normalize ${CMAKE_SOURCE_DIR}/tests/unit/test_normalize.c)
    target_link_libraries(test_normalize ct_resume_hash)
    add_test(NAME normalize COMMAND test_normalize)

    add_executable(test_hash ${CMAKE_SOURCE_DIR}/tests/unit/test_hash.c)
    target_link_libraries(test_hash ct_resume_hash)
    add_test(NAME hash COMMAND test_hash)
endif()

if(CT_RESUME_HASH_ENABLE_FUZZ)
    add_executable(fuzz_normalize ${CMAKE_SOURCE_DIR}/tests/fuzz/fuzz_normalize.c)
    target_link_libraries(fuzz_normalize ct_resume_hash)

    add_executable(fuzz_roundtrip ${CMAKE_SOURCE_DIR}/tests/fuzz/fuzz_roundtrip.c)
    target_link_libraries(fuzz_roundtrip ct_resume_hash)
endif()

if(CT_RESUME_HASH_ENABLE_BENCH)
    add_executable(bench_hash ${CMAKE_SOURCE_DIR}/benchmarks/bench_hash.c)
    target_link_libraries(bench_hash ct_resume_hash)

    add_executable(bench_normalize ${CMAKE_SOURCE_DIR}/benchmarks/bench_normalize.c)
    target_link_libraries(bench_normalize ct_resume_hash)
endif()

add_executable(dudect_runner ${CMAKE_SOURCE_DIR}/tests/timing/dudect_runner.c)
target_link_libraries(dudect_runner ct_resume_hash)

