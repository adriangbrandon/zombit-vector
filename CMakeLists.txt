cmake_minimum_required(VERSION 2.8.7)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(AppendCompilerFlags)
set(CMAKE_OSX_ARCHITECTURES x86_64)

project(succ-vectors)
set(PROJECT_VENDOR "Adrián Gómez Brandón")
set(CMAKE_PROJECT_NAME "succ-vectors")
set(PROJECT_CONTACT "adrian.gbrandon@udc.es")


#Change to "Debug" when you need it
set(CMAKE_BUILD_TYPE "Release")

# C++11 compiler Check
if(NOT CMAKE_CXX_COMPILER_VERSION) # work around for cmake versions smaller than 2.8.10
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE CMAKE_CXX_COMPILER_VERSION)
endif()
if(CMAKE_CXX_COMPILER MATCHES ".*clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()
if( (CMAKE_COMPILER_IS_GNUCXX AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4.7) OR
(CMAKE_COMPILER_IS_CLANGXX AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 3.2))
    message(FATAL_ERROR "Your C++ compiler does not support C++11. Please install g++ 4.7 (or greater) or clang 3.2 (or greater)")
else()
    message(STATUS "Compiler is recent enough to support C++11.")
endif()
if( CMAKE_COMPILER_IS_GNUCXX )
    append_cxx_compiler_flags("-std=c++11 -Wall -Wextra " "GCC" CMAKE_CXX_FLAGS)
    append_cxx_compiler_flags("-msse4.2 -O3 -ffast-math -funroll-loops -fno-omit-frame-pointer -g" "GCC" CMAKE_CXX_FLAGS_RELEASE)
else()
    append_cxx_compiler_flags("-std=c++11" "CLANG" CMAKE_CXX_FLAGS)
    append_cxx_compiler_flags("-stdlib=libc++" "CLANG" CMAKE_CXX_FLAGS)
    append_cxx_compiler_flags("-msse4.2 -O3  -ffast-math -funroll-loops -DNDEBUG" "CLANG" CMAKE_CXX_FLAGS_RELEASE)
endif()
include(CheckSSE)
FindSSE ()
if( SSE4_2_FOUND )
    if( CMAKE_COMPILER_IS_GNUCXX )
        append_cxx_compiler_flags("-msse4.2" "GCC" CMAKE_CXX_FLAGS)
    else()
        append_cxx_compiler_flags("-msse4.2" "CLANG" CMAKE_CXX_FLAGS)
    endif()
    message(STATUS "CPU does support SSE4.2.")
else()
    message(STATUS "CPU does NOT support SSE4.2")
endif()

add_subdirectory(external/sdsl-lite)
add_subdirectory(external/malloc_count)

include_directories("${PROJECT_SOURCE_DIR}/external/googletest/include"
        ${CMAKE_HOME_DIRECTORY}/include
        ${CMAKE_BINARY_DIR}/external/sdsl-lite/include
        ${CMAKE_BINARY_DIR}/external/sdsl-lite/external/libdivsufsort/include/
        ${CMAKE_HOME_DIRECTORY}/external/googletest/include
        ${CMAKE_HOME_DIRECTORY}/external/sdsl-lite/external/FastPFor/headers/
        ${CMAKE_HOME_DIRECTORY}/external/malloc_count/include
        ${STXXL_INCLUDE_DIRS}
        )


add_executable(template_executable src/main.cpp)
target_link_libraries(template_executable sdsl divsufsort divsufsort64)

add_executable(s src/s.cpp)
target_link_libraries(s sdsl divsufsort divsufsort64)

add_executable(exp-wt exp/exp-wt.cpp)
target_link_libraries(exp-wt sdsl divsufsort divsufsort64)

add_executable(exp-wm exp/exp-wm.cpp)
target_link_libraries(exp-wm sdsl divsufsort divsufsort64)

add_executable(full-wm exp/full-wm.cpp)
target_link_libraries(full-wm sdsl divsufsort divsufsort64)

add_executable(error src/error.cpp)
target_link_libraries(error sdsl divsufsort divsufsort64)

add_executable(test_partitioned_zombit test/partitioned_zombit.cpp)
target_link_libraries(test_partitioned_zombit sdsl divsufsort divsufsort64)

add_executable(test_rec_partitioned_zombit test/rec_partitioned_zombit.cpp)
target_link_libraries(test_rec_partitioned_zombit sdsl divsufsort divsufsort64)

add_executable(test_rrr test/rrr.cpp)
target_link_libraries(test_rrr sdsl divsufsort divsufsort64)

add_executable(test_oz test/oz.cpp)
target_link_libraries(test_oz sdsl divsufsort divsufsort64)

add_executable(test_sd test/sd.cpp)
target_link_libraries(test_sd sdsl divsufsort divsufsort64)


add_executable(test_zombit_s test/zombit-s.cpp)
target_link_libraries(test_zombit_s sdsl divsufsort divsufsort64)

add_executable(test_hyb test/hyb.cpp)
target_link_libraries(test_hyb sdsl divsufsort divsufsort64)

add_executable(wm exp/wm.cpp)
target_link_libraries(wm sdsl divsufsort divsufsort64)

add_executable(wt_huff exp/wt_huff.cpp)
target_link_libraries(wt_huff sdsl divsufsort divsufsort64)

add_executable(wm-bwt exp/wm-bwt.cpp)
target_link_libraries(wm-bwt sdsl divsufsort divsufsort64)

add_executable(bvs-bwt exp/bvs-bwt.cpp)
target_link_libraries(bvs-bwt sdsl divsufsort divsufsort64)

add_executable(wm-query exp/wm-query.cpp)
target_link_libraries(wm-query sdsl divsufsort divsufsort64)

add_executable(test-dac src/test-dac.cpp)
target_link_libraries(test-dac sdsl divsufsort divsufsort64)

add_executable(gen-bitvectors benchmark/util/gen-bitvectors.cpp)
target_link_libraries(gen-bitvectors sdsl divsufsort divsufsort64)

add_executable(gen-queries benchmark/util/gen-queries.cpp)
target_link_libraries(gen-queries sdsl divsufsort divsufsort64)

add_executable(exp1-build benchmark/exp1/exp1-build.cpp)
target_link_libraries(exp1-build sdsl divsufsort divsufsort64)

add_executable(exp1-bench benchmark/exp1/exp1-bench.cpp)
target_link_libraries(exp1-bench sdsl divsufsort divsufsort64)

add_executable(exp1-bench-select benchmark/exp1/exp1-bench-select.cpp)
target_link_libraries(exp1-bench-select sdsl divsufsort divsufsort64)

add_executable(exp-rep-bwt benchmark/exp-repdata/build-bwt.cpp)
target_link_libraries(exp-rep-bwt sdsl divsufsort divsufsort64)

add_executable(exp-rep-bwt-int benchmark/exp-repdata/build-bwt-int.cpp)
target_link_libraries(exp-rep-bwt-int sdsl divsufsort divsufsort64)

add_executable(exp-rep-wm benchmark/exp-repdata/build-wm.cpp)
target_link_libraries(exp-rep-wm sdsl divsufsort divsufsort64)

add_executable(exp-rep-bench benchmark/exp-repdata/bench-wm.cpp)
target_link_libraries(exp-rep-bench sdsl divsufsort divsufsort64)

add_executable(exp-rep-bvs benchmark/exp-repdata/build-bvs.cpp)
target_link_libraries(exp-rep-bvs sdsl divsufsort divsufsort64)

add_executable(exp-rep-wm-level benchmark/exp-repdata/build-wm-level.cpp)
target_link_libraries(exp-rep-wm-level sdsl divsufsort divsufsort64)

add_executable(exp-roaring-bvs benchmark/exp-roaring/build-roaring-bvs.cpp)
target_link_libraries(exp-roaring-bvs sdsl divsufsort divsufsort64)

add_executable(exp-bench-roaring-bvs benchmark/exp-roaring/bench-roaring-bvs.cpp)
target_link_libraries(exp-bench-roaring-bvs sdsl divsufsort divsufsort64)

add_executable(bitvector2roaring benchmark/util/bitvector2roaring.cpp)
target_link_libraries(bitvector2roaring sdsl divsufsort divsufsort64)

add_executable(intersection test/intersection.cpp)
target_link_libraries(intersection sdsl divsufsort divsufsort64)

add_executable(build_traj benchmark/exp-trajectories/build-traj.cpp)
target_link_libraries(build_traj sdsl divsufsort divsufsort64)

add_executable(traj-build-bvs benchmark/exp-trajectories/traj-build-bvs.cpp)
target_link_libraries(traj-build-bvs sdsl divsufsort divsufsort64)

add_executable(bench-traj benchmark/exp-trajectories/bench-traj.cpp)
target_link_libraries(bench-traj sdsl divsufsort divsufsort64)

add_executable(stats-traj benchmark/exp-trajectories/stats-traj.cpp)
target_link_libraries(stats-traj sdsl divsufsort divsufsort64)