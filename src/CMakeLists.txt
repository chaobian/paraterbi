list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")



# link with my own boost library in case we want thread sanitizers (this is necessary for boost::lockfree)
if("${FSAN}" STREQUAL "thread")
  set(Boost_NO_SYSTEM_PATHS ON)
  set(BOOST_ROOT "${BOOST160_TSAN_ROOT}")
  set(BOOST_LIBRARY_DIR "${BOOST160_TSAN_ROOT}/lib")
  find_package(Boost 1.60.0 COMPONENTS system   thread  REQUIRED)
elseif("${FSAN}" STREQUAL "address")
  set(Boost_NO_SYSTEM_PATHS ON)
  set(BOOST_ROOT "${BOOST160_ASAN_ROOT}")
  set(BOOST_LIBRARY_DIR "${BOOST160_ASAN_ROOT}/lib")
  find_package(Boost 1.60.0 COMPONENTS system   thread  REQUIRED)
else()
  find_package(Boost 1.54.0 COMPONENTS system    thread  REQUIRED)
endif()

find_package(Threads REQUIRED)
find_package(Vc REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${VC_INCLUDE_DIR})
include_directories(.)
include_directories(include)

file(GLOB ECHO_SRC_FILES
  "*.cpp"
#  "*.hpp"
#  "include/*.hpp"
)


add_executable(paraterbi_baseline main.cpp
${ECHO_SRC_FILES} )
target_link_libraries(paraterbi_baseline profiler)
target_link_libraries(paraterbi_baseline ${Boost_LIBRARIES})
set_target_properties(paraterbi_baseline 
PROPERTIES
#COMPILE_FLAGS "-DVITERBI_BASELINE=1"
COMPILE_DEFINITIONS VITERBI_DEVEL_ITERATION=1
RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})


add_executable(paraterbi_simd main.cpp
${ECHO_SRC_FILES} )
target_link_libraries(paraterbi_simd profiler)
target_link_libraries(paraterbi_simd ${Boost_LIBRARIES})
target_link_libraries(paraterbi_simd ${Vc_LIBRARIES})
set_target_properties(paraterbi_simd 
PROPERTIES 
#COMPILE_FLAGS "-DVITERBI_SIMD=1"
COMPILE_DEFINITIONS VITERBI_DEVEL_ITERATION=2
RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_executable(paraterbi main.cpp
${ECHO_SRC_FILES} )
target_link_libraries(paraterbi profiler)
target_link_libraries(paraterbi ${Boost_LIBRARIES})
target_link_libraries(paraterbi ${Vc_LIBRARIES})
target_link_libraries(paraterbi ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(paraterbi 
  PROPERTIES 
#COMPILE_FLAGS "-DVITERBI_FULL=1"
COMPILE_DEFINITIONS VITERBI_DEVEL_ITERATION=3
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

